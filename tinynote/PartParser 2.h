//
// Created by mqp on 5/21/22.
//

#ifndef AIDA_PARTPARSER_H
#define AIDA_PARTPARSER_H

#include <tao/pegtl.hpp>

#include "tinynote/MeasureParser.h"
#include "m32/Part.h"

/*
 * MeasuredPart[attributes] {
 *      Mea[] {},
 *      Mea {}
 * }
 *
 * Part[attributes] {
 *      <simple notes>,
 *      <tuples>
 * }
 * */

namespace tinynote {
    using namespace tao::pegtl;

    using TimeAssign = MeasureParserInternal::TimeAssign;
    using ClefAssign = MeasureParserInternal::ClefAssign;
    using KeyAssign = MeasureParserInternal::KeyAssign;

    namespace MeasuredPartParserInternal {

        struct MeasuredPart
                : if_must <
                    string<'M', 'e', 'a', 's', 'u', 'r', 'e', 'd', 'P', 'a', 'r', 't'>,
                    seq <
                        opt< BracketedListOf< MeasureParserInternal::MeasureAttrib > >,
                        BracketedListOf < MeasureParserInternal::Measure, '{', '}'>
                    >
                > {};

        template <typename Rule>
        struct MeaPartParseAction : nothing<Rule> {};

        template <>
        struct MeaPartParseAction< MeasureParserInternal::Measure >
        {
            template <typename ActionInput, typename MeaPartT>
            static void apply(const ActionInput& in, MeaPartT& mpart)
            {
                auto measure = tinynote::parse_measure<
                        m32::Offset, m32::Duration,
                        m32::GNote<m32::Offset, m32::Duration>
                        > (in.string());

                measure.measure_number = mpart.measures.size() + 1;
                mpart.measures.push_back(std::move(measure));
                if (!mpart.measures.empty()) {
                    const auto& first_measure = mpart.measures.front();
                    mpart.key_sig = first_measure.mea_key_sig.template value_or(mpart.key_sig);
                    mpart.clef_sign = first_measure.mea_clef_sign.template value_or(mpart.clef_sign);
                    mpart.time_sig = first_measure.mea_time_sig.value_or(mpart.time_sig);
                }
            }
        };

        inline m32::MeasuredPart<m32::Offset, m32::Duration, m32::GNote<m32::Offset, m32::Duration>>
        parse_measured_part(std::string in) {
            boost::erase_all(in, "\n");
            string_input inp (in, "parse_measured_part");
            return []<typename ParseInput>(ParseInput& in) {
                auto mpart = m32::MeasuredPart<m32::Offset, m32::Duration, m32::GNote<m32::Offset, m32::Duration>>("tinynote", 0, 'G', {4, 4});
                parse<MeasuredPart, MeaPartParseAction>(in, mpart);
                return mpart;
            }(inp);
        }
    }

    // basically a big measure
    namespace PartParserInternal {
        struct PartSimpleNote : SimpleNoteInternal::SimpleNote {};
        struct PartTuplet: TupletInternal::Tuplet{};

        struct PartElement
                : sor<
                        PartTuplet,
                        PartSimpleNote
                > {};

        struct Part
                : if_must <
                    string<'P', 'a', 'r', 't'>,
                    seq <
                        opt< BracketedListOf<MeasureParserInternal::MeasureAttrib, '[', ']'> >,
                        BracketedListOf<PartElement, '{', '}'>
                    >
                >{};

        template <typename Rule>
        struct PartParserAction : nothing<Rule> {};

        template <>
        struct PartParserAction<BracketedListOf<MeasureParserInternal::MeasureAttrib, '[', ']'>>
        {
            template <typename ActionInput, typename PartT>
            static void apply(const ActionInput &in, PartT& part)
            {
                auto [clef, key, time, start, length] = MeasureParserInternal::parse_measure_attribs(in.string());

                part.clef_sign = clef.value();
                part.key_sig = key.value();
                part.time_sig = time.value();
            }
        };

        template <>
        struct PartParserAction<PartSimpleNote>
        {
            template <typename ActionInput, typename PartT>
            static void apply(const ActionInput &in, PartT& part)
            {
                auto snote = parse_simple_note(in.string());
                snote.template displace_start<KeepLength>(
                        part.gnotes.empty()
                        ? 0
                        : part.gnotes.back().get_end()
                );
                part.gnotes.push_back(std::move(snote));
            }
        };

        template <>
        struct PartParserAction<PartTuplet>
        {
            template <typename ActionInput, typename PartT>
            static void apply(const ActionInput &in, PartT& part)
            {
                std::string str = in.string();
                auto tuplet = parse_tuplet(str);
                tuplet.template displace_start<KeepLength>(
                        part.gnotes.empty()
                        ? 0
                        : part.gnotes.back().get_end()
                );
                part.gnotes.push_back(std::move(tuplet));
            }
        };

        template<typename OffsetT, typename DurationT, typename GNoteT = m32::GNote<OffsetT, DurationT>>
        inline m32::Part<GNoteT>
        parse_part(std::string in) {
            boost::erase_all(in, "\n");
            string_input inp(in, "parse_part");
            auto part = m32::Part<GNoteT>("tinynote", 0, 'G', {4, 4});
            parse<Part, PartParserAction> (inp, part);
            return part;
        }
    }

    using namespace MeasuredPartParserInternal;
    using namespace PartParserInternal;
}

#endif //AIDA_PARTPARSER_H
