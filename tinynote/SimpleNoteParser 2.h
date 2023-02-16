//
// Created by dop on 12/27/21.
//

#ifndef AIDA_SIMPLENOTEPARSER_H
#define AIDA_SIMPLENOTEPARSER_H

#include <tao/pegtl.hpp>
#include <boost/algorithm/string.hpp>

#include "m32/SimpleNote.h"
#include "m32/Lyric.h"
#include "m32/Color.h"
#include "m32/types.h"

#include "PitchParser.h"
#include "ParseException.h"
#include "tinynote/common.h"
#include "tinynote/Length.h"

/*
 * SingleNote: <Pitch>:<length>[tie_info=TieStart/TieEnd, lyrics=["sdfs", "sdr"], color=#2A4BFF, start=<offset>]
 * */

namespace tinynote {
    using namespace tao::pegtl;
    namespace SimpleNoteInternal {

        struct SharpHex: seq< one<'#'>,  until< not_at< xdigit > > > {};
        struct ColorAttribAssignment
            : seq <
                string<'c', 'o', 'l', 'o', 'r'>,
                one< '=' >,
                SharpHex
            > {};

        struct TieStartStr : string<'T', 'i', 'e', 'S', 't', 'a', 'r', 't'>{};
        struct TieNeitherStr : string<'T', 'i', 'e', 'N', 'e', 'i', 't', 'h', 'e', 'r'>{};
        struct TieEndStr : string<'T', 'i', 'e', 'E', 'n', 'd'>{};
        struct TieInfoAttribAssignment
            : seq <
                string<'t', 'i', 'e', '_', 'i', 'n', 'f', 'o'>,
                one< '=' >,
                list_must<
                    sor<
                        TieStartStr,
                        TieNeitherStr,
                        TieEndStr
                    >,
                    one<'+'>
                >
            > {};

        struct LyricString : QuotedString{};
        struct LyricAttribAssignment
            : seq <
                string<'l', 'y', 'r', 'i', 'c', 's'>,
                one< '=' >,
                BracketedListOf< LyricString >
            > {};

        struct StartOffset: tinynote::FloatOrFraction{};
        struct StartOffsetAssignment
            : seq <
                string<'s', 't', 'a', 'r', 't'>,
                one< '=' >,
                must< StartOffset >
            > {};

        using BracketedListOfAttribAssignment = BracketedListOf<
                    sor<
                        ColorAttribAssignment,
                        TieInfoAttribAssignment,
                        LyricAttribAssignment,
                        StartOffsetAssignment >
                >;

        struct Rest:
            seq <
                string<'r', 'e', 's', 't'>
            > {};

        struct GNoteLength: tinynote::Length{};
        struct SingleNote
            : seq<
                sor< tinynote::PitchInternal::Pitch, Rest >,
                one<':'>,
                GNoteLength,
                opt< BracketedListOfAttribAssignment >
            > {};

        struct Chord
            : seq <
                string<'C', 'h'>,
                BracketedListOf < tinynote::PitchInternal::Pitch >,
                one< ':' >,
                GNoteLength,
                opt < BracketedListOfAttribAssignment >
            > {};

        struct SimpleNote
            : sor< Chord, SingleNote > {};

        template<typename Rule>
        struct SimpleNoteParseAction : nothing< Rule > {};

        template<>
        struct SimpleNoteParseAction<tinynote::PitchInternal::Pitch> {
            template<typename ActionInput, typename OffsetType, typename I>
            static void apply(const ActionInput &in,
                              m32::SimpleNote<OffsetType, boost::rational<I>> &single_note)
                              {
                single_note.pitches.push_back(std::move(parse_pitch(in.string())));
            }
        };

        template<>
        struct SimpleNoteParseAction<Rest> {
            template<typename ActionInput, typename OffsetType, typename I>
            static void apply(const ActionInput &in,
                              m32::SimpleNote<OffsetType, boost::rational<I>> &single_note) {}
        };

        template<>
        struct SimpleNoteParseAction<GNoteLength> {
            template<typename ActionInput, typename m32_SimpleNote>
            static void apply(const ActionInput &in, m32_SimpleNote& single_note)
            {
                decltype(single_note.length) length;
                length = parse_float_or_fraction<decltype(length)>(in.string());
                single_note.template set_length<KeepStart>(length);
            }
        };

        template<>
        struct SimpleNoteParseAction<TieStartStr> {
            template<typename ActionInput, typename OffsetType, typename I>
            static void apply(const ActionInput &in, m32::SimpleNote<OffsetType, boost::rational<I>> &single_note) {
                single_note.tie_info |= m32::TieStart;
            }
        };

        template<>
        struct SimpleNoteParseAction<TieEndStr> {
            template<typename ActionInput, typename OffsetType, typename I>
            static void apply(const ActionInput &in, m32::SimpleNote<OffsetType, boost::rational<I>> &single_note) {
                single_note.tie_info |= m32::TieEnd;
            }
        };

        template<>
        struct SimpleNoteParseAction<TieNeitherStr> {
            template<typename ActionInput, typename OffsetType, typename I>
            static void apply(const ActionInput &in, m32::SimpleNote<OffsetType, boost::rational<I>> &single_note) {
                single_note.tie_info |= m32::TieNeither;
            }
        };

        template<>
        struct SimpleNoteParseAction<LyricString> {
            template<typename ActionInput, typename OffsetType, typename I>
            static void apply(const ActionInput &in, m32::SimpleNote<OffsetType, boost::rational<I>> &single_note)
            {
                std::string lyric_str = in.string();
                boost::erase_all(lyric_str, "\"");
                single_note.append_lyric(lyric_str);
            }
        };

        template<>
        struct SimpleNoteParseAction<SharpHex> {
            template<typename ActionInput, typename OffsetType, typename I>
            static void apply(const ActionInput &in, m32::SimpleNote<OffsetType, boost::rational<I>> &single_note)
            { single_note.color = m32::Color::from_hex_rgb(in.string()); }
        };

        template<>
        struct SimpleNoteParseAction<StartOffset> {
            template<typename ActionInput, typename OffsetType, typename I>
            static void apply(const ActionInput &in, m32::SimpleNote<OffsetType, boost::rational<I>> &single_note)
            {
                decltype(single_note.start) start;
                start = parse_float_or_fraction<boost::rational<I>>(in.string());
                single_note.template set_start<KeepLength>(start);
            }
        };
    }

    inline m32::SimpleNote<m32::Duration, m32::Duration>
    parse_simple_note(std::string in) {
        string_input inp (in, "d");
        return []<typename ParseInput>(ParseInput& in) {
            m32::SimpleNote<m32::Duration, m32::Duration> single_note (0, 0, {}, std::nullopt, m32::TieNeither);
            parse<SimpleNoteInternal::SimpleNote, SimpleNoteInternal::SimpleNoteParseAction> (in, single_note);
            return single_note;
        }(inp);
    }

}

#endif //AIDA_SIMPLENOTEPARSER_H
