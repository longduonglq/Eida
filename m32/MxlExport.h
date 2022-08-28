// Project AIDA
// Created by Long Duong on 7/10/22.
// Purpose: 
//

#ifndef AIDA_MXLEXPORT_H
#define AIDA_MXLEXPORT_H

#include <iostream>
#include <exception>
#include <sstream>
#include <fmt/format.h>
#include <boost/filesystem.hpp>

#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "Score.h"
#include "Part.h"
#include "Pitch.h"
#include "types.h"

#define NEW_NODE(parent_node, node_ident, node_name)                                        \
    xml_node<>* node_ident = score_doc.allocate_node(node_type::node_element, node_name);   \
    parent_node->append_node(node_ident);                                                   \

#define ADD_DATA_TO_NODE(node, data_cstr) {node->append_node(score_doc.allocate_node(node_type::node_data, "", score_doc.allocate_string(data_cstr)));}

#define ADD_ATTRIBUTE_TO_NODE(node, key, value) {node->append_attribute(score_doc.allocate_attribute(key, score_doc.allocate_string(value)));}

namespace m32 {
    bool inline compare_xml_node(rapidxml::xml_node<>* self, rapidxml::xml_node<>* other) {
        std::vector<char> ss1, ss2;
        rapidxml::print(std::back_inserter(ss1), *self);
        rapidxml::print(std::back_inserter(ss2), *other);

        if (ss1.size() == ss2.size() && std::equal(ss1.begin(), ss1.end(), ss2.begin())) return true;
        else {
            auto [it1, it2] = std::mismatch(ss1.begin(), ss1.end(), ss2.begin());
            fmt::print("Mismatch between {} and {}. Distances are {} and {}.\n", *it1, *it2, std::distance(ss1.begin(), it1), std::distance(ss2.begin(), it2));
            std::string_view ctx1 (std::clamp(it1 - 20, ss1.begin(), ss1.end()), std::clamp(it1 + 20, ss1.begin(), ss1.end()));
            std::string_view ctx2 (std::clamp(it2 - 20, ss2.begin(), ss2.end()), std::clamp(it2 + 20, ss2.begin(), ss2.end()));
            fmt::print("Context:\n {} \n and {} \n", ctx1, ctx2);
            return false;
        }
    }

    using namespace rapidxml;

    class MxlExportException : public std::exception {
    public:
        template <typename ... Args>
        explicit MxlExportException(std::string_view fmt, Args... args)
                : msg (fmt::vformat(fmt, fmt::make_format_args(args...)))
        {}
        virtual const char* what() const noexcept {return msg.c_str();}
    private:
        std::string msg;
    };

    template <typename MeasuredScoreT, bool encode_date = true>
    struct MxlExporter {
        rapidxml::xml_document<> score_doc;
        MeasuredScoreT* mscore;
        using MeasuredPartT = typename MeasuredScoreT::MPartT;
        using OffsetT = typename MeasuredPartT::MeasureT::OffsetT;
        using DurationT = typename MeasuredPartT::MeasureT::DurationT;
        using GNoteT = typename MeasuredPartT::MeasureT::GNoteT;
        using TupletT = typename GNoteT::TupletT;
        using SimpleNoteT = typename GNoteT::SimpleNoteT;

        MxlExporter()
        {
        }

        enum TupletNotationState {
            TupletNone =0,
            TupletStart =1,
            TupletStop =2
        };

        void xml_to_path(const char* path, MeasuredScoreT& mscore) {
            std::ofstream fout(path);
            xml_to_stream(fout, mscore);
        }

        void xml_to_stream(std::ostream& os, MeasuredScoreT& mscore) {
            build_xml_doc(mscore);
            os << score_doc << std::endl;
        }

        xml_document<>& build_xml_doc(MeasuredScoreT& mscore) {
            this->mscore = std::addressof(mscore);
            xml_node<>* score_partwise = score_doc.allocate_node(node_type::node_element, "score-partwise");
            xml_attribute<>* version_attr = score_doc.allocate_attribute("version", "3.1");
            score_partwise->append_attribute(version_attr);
            score_doc.append_node(score_partwise);

            add_metadata(score_partwise);
            for (auto part_it = this->mscore->measured_parts.begin(); part_it != this->mscore->measured_parts.end(); part_it ++) {
//                if (std::distance(this->mscore->measured_parts.begin(), part_it) == 1) {
//                    std::cout << "" << std::endl;
//                }
                add_part(score_partwise, fmt::format("P{}", 1 + std::distance(this->mscore->measured_parts.begin(), part_it)).c_str(), *part_it);
            }
            return score_doc;
        }

        void reset() {
            score_doc.clear();
            mscore = nullptr;
        }

        void add_metadata(xml_node<>* score_pw) {
            // <work>
            NEW_NODE(score_pw, work_tag, "work")
            {
                NEW_NODE(work_tag, worktitle_tag, "work-title")
                {
                    ADD_DATA_TO_NODE(worktitle_tag, mscore->work_title.c_str());
                }
            }
            // <identification>
            NEW_NODE(score_pw, ident_tag, "identification")
            {
                NEW_NODE(ident_tag, creator_tag, "creator")
                {
                    ADD_ATTRIBUTE_TO_NODE(creator_tag, "type", "composer");
                    ADD_DATA_TO_NODE(creator_tag, "LDAida");
                }
                NEW_NODE(ident_tag, encoding_tag, "encoding")
                {
                    if constexpr(encode_date) {
                        NEW_NODE(encoding_tag, encodingdate_tag, "encoding-date")
                        {
                            ADD_DATA_TO_NODE(encodingdate_tag, boost::gregorian::to_iso_extended_string(
                                    boost::gregorian::day_clock::local_day()).c_str());
                        }
                    }
                    NEW_NODE(encoding_tag, software_tag, "software")
                    {
                        ADD_DATA_TO_NODE(software_tag, "Aida");
                    }
                }
            }
            // <part-list>
            NEW_NODE(score_pw, partlist_tag, "part-list")
            {
                int i = 1;
                for (const auto& measured_part : mscore->measured_parts) {
                    NEW_NODE(partlist_tag, score_part, "score-part")
                    {
                        ADD_ATTRIBUTE_TO_NODE(score_part, "id", fmt::format("P{}", i++).c_str())
                        NEW_NODE(score_part, partname_tag, "part-name")
                        {
                            ADD_DATA_TO_NODE(partname_tag, measured_part.part_name.c_str())
                        }
                    }
                }
            }
        }

        void add_part(xml_node<>* score_pw, const char* part_id, MeasuredPartT& mpart) {
            DivisionType common_division = 1;
            for (auto& measure: mpart.measures) {
                for (auto& gnote: measure.gnotes) {
                    if (gnote.is_tuplet()) {
                        auto& tuplet = gnote.as_tuplet();
                        if (!tuplet.is_packed()) tuplet.pack();
                        for (const auto& tup: tuplet.notes) {
                            common_division = std::lcm(common_division, tup.length.denominator());
                        }
                    }
                    common_division = std::lcm(common_division, gnote.get_length().denominator());
                }
            }

            NEW_NODE(score_pw, part_tag, "part") {
                ADD_ATTRIBUTE_TO_NODE(part_tag, "id", part_id);
                for (auto mea_it = mpart.measures.begin(); mea_it != mpart.measures.end(); mea_it ++) {
                    NEW_NODE(part_tag, measure_tag, "measure")
                    {
                        ADD_ATTRIBUTE_TO_NODE(measure_tag, "number", fmt::format("{}", mea_it->measure_number).c_str())
                        // <attributes>
                        if (std::distance(mpart.measures.begin(), mea_it) == 0) {
                            NEW_NODE(measure_tag, attributes_tag, "attributes")
                            {
                                NEW_NODE(attributes_tag, divisions_tag, "divisions")
                                {
                                    ADD_DATA_TO_NODE(divisions_tag, fmt::format("{}", common_division).c_str())
                                }
                                NEW_NODE(attributes_tag, key_tag, "key")
                                {
                                    NEW_NODE(key_tag, fifths_tag, "fifths")
                                    {
                                        ADD_DATA_TO_NODE(fifths_tag, fmt::format("{}", mea_it->mea_key_sig.has_value() ? mea_it->mea_key_sig.value(): mpart.key_sig).c_str())
                                    }
                                }
                                NEW_NODE(attributes_tag, time_tag, "time")
                                {
                                    NEW_NODE(time_tag, beats, "beats")
                                    {
                                        ADD_DATA_TO_NODE(beats, fmt::format("{}", mea_it->mea_time_sig.has_value() ? mea_it->mea_time_sig.value().first : mpart.time_sig.first).c_str())
                                    }
                                    NEW_NODE(time_tag, beat_type, "beat-type")
                                    {
                                        ADD_DATA_TO_NODE(beat_type, fmt::format("{}", mea_it->mea_time_sig.has_value() ? mea_it->mea_time_sig.value().second : mpart.time_sig.second).c_str())
                                    }
                                }
                                NEW_NODE(attributes_tag, clef_tag, "clef")
                                {
                                    NEW_NODE(clef_tag, sign_tag, "sign")
                                    {
                                        ADD_DATA_TO_NODE(sign_tag, fmt::format("{}", mea_it->mea_clef_sign.has_value() ? mea_it->mea_clef_sign.value() : mpart.clef_sign).c_str())
                                    }
                                }
                            }
                        }
                        for (auto& gnote : mea_it->gnotes) {
                            if (gnote.is_simple_note()) {
                                add_simple_note(measure_tag, gnote.as_simple_note(), common_division);
                            }
                            else {
                                auto& tuplet = gnote.as_tuplet();
                                if (!tuplet.is_packed()) tuplet.pack();
                                add_tuplet_note(measure_tag, tuplet, common_division);
                            }
                        }
                    }
                }
            }
        }

        void add_tuplet_note(
                xml_node<>* measure_tag,
                TupletT& tuplet,
                DivisionType divisions)
        {
            for (auto tuplet_component = tuplet.notes.begin(); tuplet_component != tuplet.notes.end(); tuplet_component++) {
                auto time_mod = std::pair<typename TupletT::NormalNumberType, typename TupletT::NormalNumberType>(tuplet.actual_number, tuplet.normal_number);
                auto detupletized_duration = tuplet_component->length * tuplet.actual_number / tuplet.normal_number;

                bool is_first_tup_in_tuplet = tuplet_component == tuplet.notes.begin();
                bool is_last_tup_in_tuplet = tuplet_component == std::prev(tuplet.notes.end());
                bool is_middle_tup_in_tuplet = (!is_first_tup_in_tuplet && !is_last_tup_in_tuplet);

                auto decomp = decompose_duration_into_primitives(detupletized_duration);
                for (auto dec = decomp.begin(); dec != decomp.end(); dec ++) {
                    auto is_first_fragment = dec == decomp.begin();
                    auto is_last_fragment = dec == std::prev(decomp.end());
                    auto is_middle_fragments = (!is_first_fragment && !is_last_fragment);
                    auto [fragment_principal_duration, dots] = *dec;
                    auto actual_duration = compute_dotted_length(fragment_principal_duration, dots);
                    auto is_fragmented = decomp.size() > 1;

                    if (tuplet_component->is_rest()) {
                        measure_tag->append_node(produce_single_note_tag(
                                tuplet_component->color,
                                std::nullopt,
                                tuplet_component->dynamic,
                                tuplet_component->tie_info,
                                tuplet_component->lyrics,
                                actual_duration * tuplet.normal_number / tuplet.actual_number,
                                divisions,
                                duration_name_from_duration(fragment_principal_duration),
                                dots,
                                true, // is_rest?
                                false, // is_part_of_chord
                                time_mod,
                                [&]() {
                                    if (is_first_tup_in_tuplet) return TupletNotationState::TupletStart;
                                    if (is_last_tup_in_tuplet) return TupletNotationState::TupletStop;
                                    if (is_middle_tup_in_tuplet) return TupletNotationState::TupletNone;
                                }()
                        ));
                    }
                    else {
                        for (auto pitch = tuplet_component->pitches.begin(); pitch != tuplet_component->pitches.end(); pitch ++) {
                            bool is_first_note_of_chord = pitch == tuplet_component->pitches.begin();
                            measure_tag->append_node(produce_single_note_tag(
                                    tuplet_component->color,
                                    *pitch,
                                    tuplet_component->dynamic,
                                    [&](){
                                        if (!is_fragmented) return tuplet_component->tie_info; // simple forward tie_info
                                        else {
                                            if (is_first_fragment) return tuplet_component->tie_info | TieInfo::TieStart;
                                            else if (is_last_fragment) return tuplet_component->tie_info | TieInfo::TieEnd;
                                            else if (is_middle_fragments) return tuplet_component->tie_info | TieInfo::TieStart | TieInfo::TieEnd;
                                            else assert(false);
                                        }
                                    }(),
                                    tuplet_component->lyrics,
                                    actual_duration * tuplet.normal_number / tuplet.actual_number,
                                    divisions,
                                    duration_name_from_duration(fragment_principal_duration),
                                    dots,
                                    false, // is_rest?
                                    tuplet_component->is_chord() && !is_first_note_of_chord,
                                    time_mod,
                                    [&]() {
                                        if (is_first_tup_in_tuplet) return TupletNotationState::TupletStart;
                                        if (is_last_tup_in_tuplet) return TupletNotationState::TupletStop;
                                        if (is_middle_tup_in_tuplet) return TupletNotationState::TupletNone;
                                    }()
                                    ));
                        }
                    }
                }
            }
        }

        m32::DurationName duration_name_for_tuple_with_length(m32::Duration duration) {
            return duration_name_from_duration(duration_st<Infimum, Closed>(duration).value());
        }

        void add_simple_note(
                xml_node<>* measure_tag,
                SimpleNoteT& simple_note,
                DivisionType divisions)
        {
            auto decomp = decompose_duration_into_primitives(simple_note.length);

            for (auto dec = decomp.begin(); dec != decomp.end(); dec ++) {
                auto is_first_fragment = dec == decomp.begin();
                auto is_last_fragment = dec == std::prev(decomp.end());
                auto is_middle_fragments = (!is_first_fragment && !is_last_fragment);
                auto [fragment_principal_duration, dots] = *dec;
                auto actual_duration = compute_dotted_length(fragment_principal_duration, dots);
                auto is_fragmented = decomp.size() > 1;

                if (simple_note.is_rest()) {
                    measure_tag->append_node(produce_single_note_tag(
                            simple_note.color,
                            std::nullopt,
                            simple_note.dynamic,
                            TieInfo::TieNeither,
                            is_first_fragment ? simple_note.lyrics : decltype(simple_note.lyrics)(),
                            actual_duration,
                            divisions,
                            duration_name_from_duration(fragment_principal_duration),
                            dots,
                            true, // is_rest ?
                            false, // part_of_chord ?
                            std::nullopt,
                            TupletNone
                    ));
                    return;
                }
                for (auto pitch = simple_note.pitches.begin(); pitch != simple_note.pitches.end(); pitch ++) {
                    bool is_first_note_of_chord = pitch == simple_note.pitches.begin(); // this only makes sense if simple_note.is_chord() is true
                    measure_tag->append_node(produce_single_note_tag(
                            simple_note.color,
                            *pitch,
                            simple_note.dynamic,
                            [&](){
                                if (!is_fragmented) return simple_note.tie_info; // simple forward tie_info
                                else {
                                    if (is_first_fragment) return simple_note.tie_info | TieInfo::TieStart;
                                    else if (is_last_fragment) return simple_note.tie_info | TieInfo::TieEnd;
                                    else if (is_middle_fragments) return simple_note.tie_info | TieInfo::TieStart | TieInfo::TieEnd;
                                    else assert(false);
                                }
                            }(),
                            is_first_fragment ? simple_note.lyrics : decltype(simple_note.lyrics)(),
                            actual_duration,
                            divisions,
                            duration_name_from_duration(fragment_principal_duration),
                            dots,
                            false, // is rest
                            simple_note.is_chord() && !is_first_note_of_chord,
                            std::nullopt,
                            TupletNone
                            ));
                }
            }
        }

        inline xml_node<>* produce_single_note_tag(
                std::optional<Color> color,
                std::optional<m32::Pitch> pitch,
                std::optional<float> dynamics,
                TieInfo tie_info,
                const decltype(std::declval<SimpleNoteT>().lyrics)& lyrics,

                DurationT duration,
                DivisionType division,
                m32::DurationName displayed_duration,
                int dots,

                bool is_rest,
                bool is_part_of_chord,
                std::optional<std::pair<typename TupletT::NormalNumberType, typename TupletT::NormalNumberType>> time_mod,
                TupletNotationState tuplet_state)
        {
            xml_node<>* note_tag = score_doc.allocate_node(node_type::node_element, "note");
            // <chord>
            // <pitch>
            // <rest>
            // <duration>
            // <tie>
            // <type>
            // <dot>
            // <time-modification>
            // <notations>
            // <lyric>
            if (color.has_value()) {
                ADD_ATTRIBUTE_TO_NODE(note_tag, "color", color->to_hex_rgb().c_str());
            }
            if (is_part_of_chord) { NEW_NODE(note_tag, chord_tag, "chord") }
            if (pitch.has_value()) {
                NEW_NODE(note_tag, pitch_tag, "pitch") {
                    NEW_NODE(pitch_tag, step_tag, "step") {
                        ADD_DATA_TO_NODE(step_tag, m32::Pitch::string_from_diatonic_step(pitch.value().step))
                    }
                    if (pitch.value().alter != Pitch::Alter::No) {
                        NEW_NODE(pitch_tag, alter_tag, "alter") {
                            ADD_DATA_TO_NODE(alter_tag, fmt::format("{}", static_cast<int>(pitch.value().alter)).c_str())
                        }
                    }
                    NEW_NODE(pitch_tag, octave_tag, "octave") {
                        ADD_DATA_TO_NODE(octave_tag, fmt::format("{}", static_cast<int>(pitch.value().octave.value())).c_str())
                    }
                }
            }
            if (is_rest) { NEW_NODE(note_tag, rest_tag, "rest"); }
            // <duration>
            {
                DivisionType duration_in_ticks = duration.numerator() * division / duration.denominator();
                NEW_NODE(note_tag, duration_tag, "duration") {
                    ADD_DATA_TO_NODE(duration_tag, fmt::format("{}", duration_in_ticks).c_str())
                }
            }
            // <tie>
            if (tie_info != TieInfo::TieNeither) {
                if (tie_info & TieInfo::TieStart) {
                    NEW_NODE(note_tag, tie_tag, "tie") {
                        ADD_ATTRIBUTE_TO_NODE(tie_tag, "type", "start")
                    }
                }
                if (tie_info & TieInfo::TieEnd) {
                    NEW_NODE(note_tag, tie_tag, "tie") {
                        ADD_ATTRIBUTE_TO_NODE(tie_tag, "type", "stop")
                    }
                }
            }
            // <type>
            {
                NEW_NODE(note_tag, type_tag, "type") {
                    ADD_DATA_TO_NODE(type_tag, duration_str_from_name(displayed_duration))
                }
            }
            // <dot>
            {
                for (int i = 0; i < dots; i ++) {
                    NEW_NODE(note_tag, dot_tag, "dot")
                }
            }
            // <time-modification>
            {
                if (time_mod.has_value()) {
                    auto [actual, normal] = time_mod.value();
                    NEW_NODE(note_tag, timemod_tag, "time-modification") {
                        NEW_NODE(timemod_tag, actual_tag, "actual-notes") {
                            ADD_DATA_TO_NODE(actual_tag, fmt::format("{}", actual).c_str())
                        }
                        NEW_NODE(timemod_tag, normal_tag, "normal-notes") {
                            ADD_DATA_TO_NODE(normal_tag, fmt::format("{}", normal).c_str())
                        }
                    }
                }
            }
            // <notations>
            {
                // <tuplet> or <tied type="">
                {
                    if (tuplet_state != TupletNotationState::TupletNone || tie_info != TieNeither) {
                        NEW_NODE(note_tag, notations_tag, "notations")
                        {
                            if (tuplet_state != TupletNotationState::TupletNone) {
                                NEW_NODE(notations_tag, tuplet_notation_tag, "tuplet")
                                {
                                    if (tuplet_state == TupletNotationState::TupletStart) {
                                        ADD_ATTRIBUTE_TO_NODE(tuplet_notation_tag, "type", "start")
                                    } else if (tuplet_state == TupletNotationState::TupletStop) {
                                        ADD_ATTRIBUTE_TO_NODE(tuplet_notation_tag, "type", "stop")
                                    } else
                                        assert(false);
                                }
                            }

                            if (tie_info != TieNeither) {
                                NEW_NODE(notations_tag, tied_tag, "tied") {
                                    if (tie_info & TieStart) {
                                        ADD_ATTRIBUTE_TO_NODE(tied_tag, "type", "start")
                                    }
                                    if (tie_info & TieEnd) {
                                        ADD_ATTRIBUTE_TO_NODE(tied_tag, "type", "stop")
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // <lyric>
            {
                if (!lyrics.empty()) {
                    for (const auto& lyric: lyrics) {
                        NEW_NODE(note_tag, lyric_tag, "lyric") {
                            ADD_ATTRIBUTE_TO_NODE(lyric_tag, "number", std::to_string(lyric.number).c_str())
                            NEW_NODE(lyric_tag, text_tag, "text") {
                                ADD_DATA_TO_NODE(text_tag, lyric.text.c_str())
                            }
                        }
                    }
                }
            }
            return note_tag;
        }
    };
}

#endif //AIDA_MXLEXPORT_H
