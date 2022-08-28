// Project AIDA
// Created by Long Duong on 7/4/22.
// Purpose: 
//

#ifndef AIDA_MXLIMPORT_H
#define AIDA_MXLIMPORT_H

#include <utility>
#include <fstream>
#include <vector>
#include <variant>
#include <limits>
#include <exception>
#include <fmt/format.h>
#include <boost/container_hash/hash.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

#include <rapidxml.hpp>

#include "m32/types.h"
#include "Score.h"
#include "Part.h"
#include "Pitch.h"

#define FOR_X_IN_CHILDREN_OF_DO(it_name, parent_node) for(rapidxml::xml_node<>* it_name = parent_node->first_node(); it_name; it_name = it_name->next_sibling())

#define FOR_X_IN_ATTRS_OF_DO(it_name, node) for (rapidxml::xml_attribute<>* it_name = node->first_attribute(); it_name; it_name = it_name->next_attribute())

#define IS_ATTR_KEY(attr, s) (strcmp(attr->name(), s) == 0)
#define IS_NODE_NAME(node, s) (strcmp(node->name(), s) == 0)

namespace m32 {
    using namespace rapidxml;

    class MxlImportException : public std::exception {
    public:
        template <typename ... Args>
        explicit MxlImportException(std::string_view fmt, Args... args)
                : msg (fmt::vformat(fmt, fmt::make_format_args(args...)))
        {}
        virtual const char* what() const noexcept {return msg.c_str();}
    private:
        std::string msg;
    };

    template <
            typename OffsetT = m32::Offset,
            typename DurationT = m32::Duration,
            template <typename, typename...> class PitchCont = std::vector,
            template <typename, typename...> class LyricCont = std::vector,
            template <typename, typename...> class TupNoteCont = std::vector,
            template <typename, typename...> class MeaCont = std::vector,
            template <typename, typename...> class GNoteCont = std::vector,
            template <typename, typename...> class MPartCont = std::vector,
            template <typename, typename...> class PartCont = std::vector
    >
    struct MxlImporter {
        using Self = MxlImporter<OffsetT, DurationT, PitchCont, LyricCont, TupNoteCont, MeaCont, GNoteCont, MPartCont, PartCont>;

        using GNoteT = m32::GNote<OffsetT, DurationT, PitchCont, LyricCont, TupNoteCont>;
        using TupletT = typename GNoteT::TupletT ;
        using SimpleNoteT = typename GNoteT::SimpleNoteT ;
        // using ScoreT = m32::Score<GNoteT, GNoteCont, PartCont>;
        using MeasuredScoreT = m32::MeasuredScore<OffsetT, DurationT, MeaCont, GNoteCont, MPartCont>;
        // using PartT = m32::Part<GNoteT, GNoteCont>;
        using MeasuredPartT = m32::MeasuredPart<OffsetT, DurationT, GNoteT, MeaCont, GNoteCont>;
        using MeasureT = typename MeasuredPartT::MeasureT;

        MeasuredScoreT mscore = MeasuredScoreT("");
        std::deque<std::pair<MeasuredPartT*, std::string_view>> mpart_buffer= {};
        struct Attributes {
            m32::DivisionType divisions;
            m32::KeySigType key;
            m32::TimeSigType time;
            m32::ClefSignType clef;
        };
        Attributes current_attributes;

        MxlImporter()
        {
            reset();
        }

        void parse_file(const char* file_name) {
            if (!boost::filesystem::exists(file_name))
                throw MxlImportException("File {} not found", file_name);
            std::ifstream file(file_name, std::ios::ate | std::ios::binary );
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            file.setf(std::ios_base::skipws );

            std::vector<char> buffer(size + 1);
            file.read(buffer.data(), size);
            *(buffer.data() + size) = 0;
            assert(*(buffer.data() + size) == 0);

            parse_mem(buffer.data());
        }

        void parse_mem(char* buffer) {
            rapidxml::xml_document<> doc;
            doc.parse<0>(buffer);
            parse_score_partwise(doc.first_node());
        }

        void reset() {
            memset(&current_attributes, 0, sizeof(current_attributes));
            mpart_buffer.clear();
            mscore.clear();
        }

        void parse_score_partwise(xml_node<>* score_partwise) {
            assert(strcmp(score_partwise->name(), "score-partwise") == 0);
            FOR_X_IN_CHILDREN_OF_DO(node, score_partwise) {
                if (IS_NODE_NAME(node, "work")) {
                    auto work_node = node;
                    FOR_X_IN_CHILDREN_OF_DO(work_child, work_node) {
                        if (IS_NODE_NAME(work_child, "work-title")) {
                            mscore.work_title = work_child->value();
                        }
                    }
                }
                if (IS_NODE_NAME(node, "identification")) {}
                if (IS_NODE_NAME(node, "part-list")) {
                    auto partlist = node;
                    FOR_X_IN_CHILDREN_OF_DO(partlist_child, partlist) {
                        if (IS_NODE_NAME(partlist_child, "score-part")) {
                            auto score_part = partlist_child;
                            const char* part_name = nullptr;
                            const char* id = nullptr;
                            FOR_X_IN_ATTRS_OF_DO(scpart_attr, score_part) {
                                if (IS_ATTR_KEY(scpart_attr, "id")) {
                                    id = scpart_attr->value();
                                }
                            }
                            FOR_X_IN_CHILDREN_OF_DO(scpart_child, score_part) {
                                if (IS_NODE_NAME(scpart_child, "part-name")) {
                                    part_name = scpart_child->value();
                                }
                            }
                            mpart_buffer.push_back(std::make_pair(new MeasuredPartT(part_name, KeySigType(0), ClefSignType('G'), TimeSigType{4, 4}), id));
                        }
                    }
                }

                if (IS_NODE_NAME(node, "part")) { parse_part(node); }
            }

            for(auto [mpart_ptr, part_id] : mpart_buffer) {
                mscore.measured_parts.push_back(std::move(*mpart_ptr));
                delete mpart_ptr;
            }
        }

        void chord_add(SimpleNoteT& chord, SimpleNoteT& rookie) {
            assert(chord.length == rookie.length);
            std::move(
                    rookie.pitches.begin(), rookie.pitches.end(),
                    std::back_inserter(chord.pitches));
            std::move(
                    rookie.lyrics.begin(), rookie.lyrics.end(),
                    std::back_inserter(chord.lyrics));
        }

        Attributes parse_attribute_node(xml_node<>* attr_node) {
            Attributes attributes;
            memset(&attributes, 0, sizeof(Attributes));
            if (IS_NODE_NAME(attr_node, "attributes")) {
                auto attributes_tag = attr_node;
                FOR_X_IN_CHILDREN_OF_DO(attrs_child, attributes_tag) {
                    if (IS_NODE_NAME(attrs_child, "divisions"))
                        attributes.divisions = std::stol(attrs_child->value());
                    if (IS_NODE_NAME(attrs_child, "key"))
                        FOR_X_IN_CHILDREN_OF_DO(key_child, attrs_child)
                            if (IS_NODE_NAME(key_child, "fifths"))
                                attributes.key = std::stoi(key_child->value());
                    if (IS_NODE_NAME(attrs_child, "time")) {
                        auto time_tag = attrs_child;
                        FOR_X_IN_CHILDREN_OF_DO(time_child, time_tag) {
                            if (IS_NODE_NAME(time_child, "beats"))
                                attributes.time.first = std::stol(time_child->value());
                            if (IS_NODE_NAME(time_child, "beat-type"))
                                attributes.time.second = std::stol(time_child->value());
                        }
                    }
                    if (IS_NODE_NAME(attrs_child, "clef")) {
                        auto clef_tag = attrs_child;
                        FOR_X_IN_CHILDREN_OF_DO(clef_child, clef_tag) {
                            if (IS_NODE_NAME(clef_child, "sign"))
                                attributes.clef = clef_child->value()[0];
                            switch (attributes.clef) {
                                case 'G': case 'C' : case 'F': break;
                                default: throw MxlImportException("Unknown clef: {}", attributes.clef);
                            }
                        }
                    }
                }
            }
            return attributes;
        }

        void parse_part(xml_node<>* part_node) {
             const char* part_id = nullptr;
             FOR_X_IN_ATTRS_OF_DO(part_attr, part_node) {
                 if (IS_ATTR_KEY(part_attr, "id")) part_id = part_attr->value();
             }
             // Preparse to at least populate current_attributes first
             FOR_X_IN_CHILDREN_OF_DO(mea_tag, part_node) {
                 FOR_X_IN_CHILDREN_OF_DO(mea_tag_child, mea_tag) {
                     if (IS_NODE_NAME(mea_tag_child, "attributes")) {
                         current_attributes = parse_attribute_node(mea_tag_child);
                         goto current_attributes_populated;
                     }
                 }
             }
             current_attributes_populated:

             FOR_X_IN_CHILDREN_OF_DO(mea_tag, part_node) {
                 FOR_X_IN_ATTRS_OF_DO(mea_tag_attr, mea_tag) {
                     if (IS_ATTR_KEY(mea_tag_attr, "number")) {}
                 }

                 // int nb_backups = 0;
                 auto m32_part_pair = std::find_if(mpart_buffer.begin(), mpart_buffer.end(), [part_id](const auto& p){ return strcmp(p.second.data(), part_id) == 0; });
                 assert(m32_part_pair != mpart_buffer.end());
                 // std::advance(m32_part_pair, -nb_backups);
                 MeasuredPartT* m32_part = m32_part_pair->first;

                 std::deque<GNoteT> gnote_stack;

                 // Can only create measure after <attributes> had been parsed
                 m32_part->measures.emplace_back(
                         m32_part->measures.empty() ? OffsetT{0, 1} : m32_part->measures.back().end,
                         DurationT{ current_attributes.time.first * 4, current_attributes.time.second },
                         m32_part->measures.empty() ? 1 : m32_part->measures.back().measure_number + 1,
                         GNoteCont<GNoteT>());

                 FOR_X_IN_CHILDREN_OF_DO(mea_tag_child, mea_tag) {
                     if (IS_NODE_NAME(mea_tag_child, "attributes")) {
                         auto attributes_tag = mea_tag_child;
                         current_attributes = parse_attribute_node(attributes_tag);
                         m32_part->key_sig = current_attributes.key;
                         m32_part->clef_sign = current_attributes.clef;
                         m32_part->time_sig = current_attributes.time;
                     }

                     if (IS_NODE_NAME(mea_tag_child, "note")) {
                         auto note_tag = mea_tag_child;
                         const char* note_color = nullptr;

                         std::optional<Pitch::DiatonicStep> step = std::nullopt;
                         std::optional<m32::Octave> octave = std::nullopt;
                         std::optional<Pitch::Alter> alter = std::nullopt;

                         bool is_rest = false;
                         bool is_part_of_chord = false;
                         TieInfo tie_info = TieInfo::TieNeither;
                         DivisionType duration_in_ticks = 0;
                         DurationName duration_type = DurationName::unspecified;
                         int dots = 0;
                         typename GNoteT::TupletT::NormalNumberType actual_notes = 0, normal_notes = 0;
                         const char* tuplet_type = nullptr; // start or stop
                         LyricCont<m32::Lyric> lyrics;

                         FOR_X_IN_ATTRS_OF_DO(note_tag_attr, note_tag) {
                             if (IS_ATTR_KEY(note_tag_attr, "color")) { note_color = note_tag_attr->value(); }
                         }
                         FOR_X_IN_CHILDREN_OF_DO(note_tag_child, note_tag) {
                             if (IS_NODE_NAME(note_tag_child, "pitch")) {
                                 auto pitch_tag = note_tag_child;
                                 FOR_X_IN_CHILDREN_OF_DO(pitch_child, pitch_tag) {
                                     if (IS_NODE_NAME(pitch_child, "step")) {
                                         step = Pitch::diatonic_step_from_char(pitch_child->value()[0]);
                                     }
                                     if (IS_NODE_NAME(pitch_child, "alter")) {
                                         long alter_ = std::stol(pitch_child->value());
                                         if (alter_ != 1 && alter_ != -1 && alter_ != 0)
                                             throw MxlImportException("Unknown alter value: {}", alter_);
                                         alter = static_cast<Pitch::Alter>(alter_);
                                     }
                                     if (IS_NODE_NAME(pitch_child, "octave")) {
                                         octave = std::stol(pitch_child->value());
                                     }
                                 }
                             }
                             if (IS_NODE_NAME(note_tag_child, "rest")) { is_rest = true; }
                             if (IS_NODE_NAME(note_tag_child, "chord")) { is_part_of_chord = true; }
                             if (IS_NODE_NAME(note_tag_child, "tie")) {
                                 auto tie_tag = note_tag_child;
                                 FOR_X_IN_ATTRS_OF_DO(tie_attr, tie_tag) {
                                     if (IS_ATTR_KEY(tie_attr, "type")) {
                                         if (strcmp(tie_attr->value(), "start") == 0) tie_info |= TieInfo::TieStart;
                                         if (strcmp(tie_attr->value(), "stop") == 0) tie_info |= TieInfo::TieEnd;
                                     }
                                 }
                             }
                             if (IS_NODE_NAME(note_tag_child, "duration")) { duration_in_ticks = std::stol(note_tag_child->value()); }
                             if (IS_NODE_NAME(note_tag_child, "type")) { duration_type = duration_name_from_str(note_tag_child->value()); }
                             if (IS_NODE_NAME(note_tag_child, "dot")) { dots++; }
                             if (IS_NODE_NAME(note_tag_child, "accidental")) {  }
                             if (IS_NODE_NAME(note_tag_child, "time-modification")) {
                                 auto time_mod_tag = note_tag_child;
                                 FOR_X_IN_CHILDREN_OF_DO(timemod_child, time_mod_tag) {
                                     if (IS_NODE_NAME(timemod_child, "actual-notes")) {
                                         actual_notes = std::stol(timemod_child->value());
                                     }
                                     if (IS_NODE_NAME(timemod_child, "normal-notes")) {
                                         normal_notes = std::stol(timemod_child->value());
                                     }
                                 }
                             }
                             if (IS_NODE_NAME(note_tag_child, "notations")) {
                                 auto notations_tag = note_tag_child;
                                 FOR_X_IN_CHILDREN_OF_DO(notation_child, notations_tag) {
                                     if (IS_NODE_NAME(notation_child, "tuplet")) {
                                         FOR_X_IN_ATTRS_OF_DO(tuplet_attr, notation_child) {
                                             if (IS_ATTR_KEY(tuplet_attr, "type")) {
                                                 tuplet_type = tuplet_attr->value();
                                             }
                                         }
                                     }
                                 }
                             }
                             if (IS_NODE_NAME(note_tag_child, "lyric")) {
                                 auto lyric_tag = note_tag_child;
                                 FOR_X_IN_CHILDREN_OF_DO(lyric_child, lyric_tag) {
                                     if (IS_NODE_NAME(lyric_child, "text")) {
                                         lyrics.template emplace_back(
                                                 lyrics.empty() ? 0 : lyrics.size() + 1,
                                                 lyric_child->value()
                                         );
                                     }
                                 }
                             }
                             if (IS_NODE_NAME(note_tag_child, "notehead")) {
                                 FOR_X_IN_ATTRS_OF_DO(ntheadd_attr, note_tag_child) {
                                     if (IS_ATTR_KEY(ntheadd_attr, "color")) {
                                         note_color = ntheadd_attr->value();
                                     }
                                 }
                             }
                         }

                         /// Now actually build the note
                         bool is_part_of_tuplet = (actual_notes != 0 && normal_notes != 0) || tuplet_type != nullptr;
                         m32::Duration duration{duration_in_ticks, current_attributes.divisions};
                         std::optional<m32::Color> m32_color = (note_color == nullptr) ? std::nullopt : std::optional(m32::Color::from_hex_rgb(note_color));
                         auto& current_measure = m32_part->measures.back();
                         auto last_offset = current_measure.gnotes.empty() ? OffsetT{0, 1} : current_measure.gnotes.back().get_end();
                         SimpleNoteT sn(
                                 last_offset,
                                 duration,
                                 lyrics,
                                 m32_color,
                                 tie_info);
                         if (!is_rest) {
                             sn.pitches.emplace_back(
                                     step.value(),
                                     octave,
                                     alter.template value_or(Pitch::Alter::No));
                         }

                         if (!is_part_of_tuplet) {
                             if (is_part_of_chord) {
                                 SimpleNoteT& last_snote = current_measure.gnotes.back().as_simple_note();
                                 assert(last_snote.length == sn.length);
                                 std::move(
                                         sn.pitches.begin(), sn.pitches.end(),
                                         std::back_inserter(last_snote.pitches));
                             }
                             else { current_measure.gnotes.push_back(std::move(sn)); }
                         }
                         else {
                             if (actual_notes != 0 && normal_notes != 0 && tuplet_type == nullptr) {
                                 TupletT &last_tup = current_measure.gnotes.back().as_tuplet();
                                 assert(last_tup.is_tuplet());
                                 if (!is_part_of_chord) { last_tup.notes.push_back(std::move(sn)); }
                                 else {
                                     chord_add(last_tup.notes.back(), sn);
//                                     assert(last_tup.notes.back().length == sn.length);
//                                     std::move(
//                                             sn.pitches.begin(), sn.pitches.end(),
//                                             std::back_inserter(last_tup.notes.back().pitches));
                                 }
                             }
                             else if (strcmp(tuplet_type, "start") == 0) {
                                 assert(actual_notes != 0 && normal_notes != 0);
                                 if (!is_part_of_chord) {
                                     current_measure.gnotes.emplace_back(
                                             std::in_place_type<TupletT>,
                                             normal_notes,
                                             actual_notes,
                                             last_offset,
                                             duration,
                                             TupNoteCont<SimpleNoteT>{sn});
                                 }
                                 else {
                                     TupletT& last_tup = current_measure.gnotes.back().as_tuplet();
                                     chord_add(last_tup.notes.back(), sn);
//                                     std::move(
//                                             sn.pitches.begin(), sn.pitches.end(),
//                                             std::back_inserter(last_tup.notes.back().pitches));
                                 }
                             }
                             else if (strcmp(tuplet_type, "stop") == 0) {
                                 TupletT& last_tup = current_measure.gnotes.back().as_tuplet();
                                 assert(last_tup.is_tuplet());
                                 if (!is_part_of_chord) { last_tup.notes.push_back(std::move(sn)); }
                                 else {
                                     chord_add(last_tup.notes.back(), sn);
//                                     assert(last_tup.notes.back().length == sn.length);
//                                     std::move(
//                                             sn.pitches.begin(), sn.pitches.end(),
//                                             std::back_inserter(last_tup.notes));
                                 }
                             }
                             else throw MxlImportException("Unknown tuplet_type or (actual, normal): {}, ({}, {})", tuplet_type,actual_notes, normal_notes);

                             TupletT& last_tup = current_measure.gnotes.back().as_tuplet();
                             last_tup.template set_length<KeepStart>(
                                     std::transform_reduce(
                                             last_tup.notes.begin(), last_tup.notes.end(),
                                             DurationT{0, 1},
                                             std::plus<>(),
                                             [](const auto& sn){ return sn.length; }));
                         }
                     }
                     if (IS_NODE_NAME(mea_tag_child, "backup")) {
                         auto backup_tag = mea_tag_child;
                         FOR_X_IN_CHILDREN_OF_DO(backup_child, backup_tag) {
                             if (IS_NODE_NAME(backup_child, "duration")) {
                                 auto backup_duration = std::stoll(backup_child->value());
                                 assert(DurationT(backup_duration, current_attributes.divisions) == m32_part->measures.back().length);
                                 if (std::next(m32_part_pair) == mpart_buffer.end()) {
                                     mpart_buffer.push_back(std::make_pair(new MeasuredPartT(m32_part->part_name, current_attributes.key, current_attributes.clef, current_attributes.time), part_id));
                                     m32_part_pair = std::prev(mpart_buffer.end());
                                 }
                                 else m32_part_pair++;
                                 m32_part = m32_part_pair->first;

                                 m32_part->measures.emplace_back(
                                     m32_part->measures.empty() ? OffsetT{0, 1} : m32_part->measures.back().end,
                                     DurationT{current_attributes.time.first * 4, current_attributes.time.second},
                                     m32_part->measures.empty() ? 1 : m32_part->measures.back().measure_number,
                                     GNoteCont<GNoteT>());
                             }
                         }
                     }
                     if (IS_NODE_NAME(mea_tag_child, "forward")) {}
                     if (IS_NODE_NAME(mea_tag_child, "direction")) {}
                     if (IS_NODE_NAME(mea_tag_child, "barline")) {}
                 }
             }
        }
    };
}

#endif //AIDA_MXLIMPORT_H
