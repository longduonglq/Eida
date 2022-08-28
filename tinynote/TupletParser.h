//
// Created by dop on 12/27/21.
//

#ifndef AIDA_TUPLETPARSER_H
#define AIDA_TUPLETPARSER_H

#include <tao/pegtl.hpp>
#include <boost/algorithm/string.hpp>

#include "m32/Tuplet.h"
#include "m32/SimpleNote.h"
#include "m32/Lyric.h"
#include "m32/Color.h"
#include "m32/types.h"

#include "SimpleNoteParser.h"
#include "PitchParser.h"
#include "ParseException.h"
#include "tinynote/common.h"
#include "tinynote/Length.h"
#include "tinynote/Number.h"

/*
 * Tuplet: Tup@<normal_number>!<actual_number>[<SimpleNote>+]:<length>
 * */

namespace tinynote {
    using namespace tao::pegtl;
    namespace TupletInternal {
        struct TupletLength: tinynote::Length{};
        struct TupletNormalNumber : tinynote::Integer{};
        struct TupletActualNumber: tinynote::Integer{};
        struct TupletSimpleNote : tinynote::SimpleNoteInternal::SimpleNote{};
        struct TupletListOfAttribs : tinynote::SimpleNoteInternal::BracketedListOfAttribAssignment{};
        struct Tuplet
                : seq <
                    string<'T', 'u', 'p'>,
                    must< one< '@' >, TupletNormalNumber >,
                    opt< if_must < one< '!' >, TupletActualNumber > >,
                    BracketedListOf< TupletSimpleNote >,
                    one< ':' >,
                    TupletLength,
                    opt< TupletListOfAttribs >
                > {};

        template <typename Rule>
        struct TupletParserAction : nothing<Rule> {};

        template <>
        struct TupletParserAction< TupletNormalNumber > {
            template<typename ActionInput, typename m32_Tuplet, typename T>
            static void apply(const ActionInput &in, m32_Tuplet& tuplet, T&) {
                auto in_str = in.string();
                tuplet.normal_number = std::stoi(in_str);
            }
        };

        template <>
        struct TupletParserAction< TupletActualNumber > {
            template<typename ActionInput, typename m32_Tuplet, typename T>
            static void apply(const ActionInput &in, m32_Tuplet& tuplet, T& expl_actual_number)
            {
                auto in_str = in.string();
                expl_actual_number = std::stoi(in_str);
            }
        };

        template <>
        struct TupletParserAction< TupletSimpleNote > {
            template<typename ActionInput, typename m32_Tuplet, typename T>
            static void apply(const ActionInput &in, m32_Tuplet& tuplet, T&) {
                auto tup_component = parse_simple_note(in.string());
                tup_component.template set_start<KeepLength>(
                        tuplet.notes.empty()
                        ? typename m32_Tuplet::Interval::DurationT{0, 1}
                        : tuplet.notes.back().end
                        );
                tuplet.notes.push_back(tup_component);
            }
        };

        template <>
        struct TupletParserAction< TupletLength > {
            template<typename ActionInput, typename m32_Tuplet, typename T>
            static void apply(const ActionInput &in, m32_Tuplet& tuplet, T& expl_act_num) {
                using Duration = typename m32_Tuplet::Interval::DurationT;
                auto ss = in.string();
                tuplet.template set_length<KeepStart>(
                        parse_length<typename Duration::int_type>(ss));

                if (!expl_act_num.has_value())
                    tuplet.actual_number = tuplet.notes.size();
                else tuplet.actual_number = expl_act_num.value();
            }
        };

        template<>
        struct TupletParserAction<SimpleNoteInternal::StartOffsetAssignment> {
            template<typename ActionInput, typename m32_Tuplet, typename T>
            static void apply(const ActionInput &in, m32_Tuplet& tuplet, T&)
            {
                decltype(tuplet.start) start;
                parse_nested< SimpleNoteInternal::StartOffsetAssignment, LengthParseAction > (
                        in.position(),
                        string_input(in.string(), "start"),
                        start );
                tuplet.template set_start<KeepLength>(start);
            }
        };
    }

    inline m32::Tuplet<m32::Duration, m32::Duration>
    parse_tuplet(std::string in) {
        string_input inp(in, "parse_tuplet");
        return []<typename ParseInput>(ParseInput& in) {
            m32::Tuplet<m32::Duration, m32::Duration> tuplet (0, 0, 0, 0, {});
            std::optional<decltype(tuplet.actual_number)> explicit_actual_number;
            parse<TupletInternal::Tuplet, TupletInternal::TupletParserAction> (in, tuplet, explicit_actual_number);
            tuplet.pack();
            return tuplet;
        }(inp);
    }
}

#endif //AIDA_TUPLETPARSER_H
