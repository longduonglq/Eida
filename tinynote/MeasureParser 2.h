//
// Created by dop on 12/28/21.
//

#ifndef AIDA_MEASUREPARSER_H
#define AIDA_MEASUREPARSER_H

#include <tao/pegtl.hpp>
#include <boost/algorithm/string.hpp>

#include "m32/SimpleNote.h"
#include "m32/Lyric.h"
#include "m32/Color.h"
#include "m32/types.h"
#include "m32/Measure.h"

#include "TupletParser.h"
#include "PitchParser.h"
#include "ParseException.h"
#include "tinynote/SimpleNoteParser.h"
#include "tinynote/common.h"
#include "tinynote/Number.h"
#include "tinynote/DebugControl.h"

/*
 * Mea[time=4/4, clef=Treble(Bass, Alto), key=4, start=?, length=?]{ Note, Chord, Note, Tup }
 * */
namespace tinynote {
    using namespace tao::pegtl;
    namespace MeasureParserInternal {
        struct TimeValueNum : tinynote::Integer{};
        struct TimeValueDenom : tinynote::Integer{};
        struct TimeAssign
                : seq <
                    string< 't', 'i', 'm', 'e' >,
                    one < '=' >,
                    must< seq< TimeValueNum, one<'/'>, TimeValueDenom > >
                > {};

        struct TrebleStr : string< 't', 'r', 'e', 'b', 'l', 'e' >{};
        struct BassStr : string< 'b', 'a', 's', 's' >{};
        struct AltoStr : string< 'a', 'l', 't', 'o' >{};
        struct ClefAssign
                : seq <
                    string< 'c', 'l', 'e', 'f' >,
                    one < '=' >,
                    sor < TrebleStr, BassStr, AltoStr >
                > {};

        struct KeyInt : tinynote::Integer{};
        struct KeyAssign
                : seq <
                    string< 'k', 'e', 'y' >,
                    one< '=' >,
                    KeyInt
                >{};

        struct MeasureSimpleNote : SimpleNoteInternal::SimpleNote {};
        struct MeasureTuplet: TupletInternal::Tuplet{};
        struct MeasureElement
                : sor<
                    MeasureTuplet,
                    MeasureSimpleNote
                    > {};

        struct StartOffset : tinynote::FloatOrFraction{};
        struct StartOffsetAssignment
                : seq <
                        string<'s', 't', 'a', 'r', 't'>,
                        one< '=' >,
                        StartOffset
                > {};

        struct LengthDuration : tinynote::FloatOrFraction{};
        struct LengthAssignment
                : seq <
                        string<'l', 'e', 'n', 'g', 't', 'h'>,
                        one< '=' >,
                        LengthDuration
                > {};

        struct MeasureAttrib
                : sor<
                    TimeAssign,
                    ClefAssign,
                    KeyAssign,
                    StartOffsetAssignment,
                    LengthAssignment
                >{};

        struct Measure
                : if_must <
                    string<'M', 'e', 'a'>,
                    seq<
                        opt< BracketedListOf<MeasureAttrib, '[', ']'> >,
                        BracketedListOf<MeasureElement, '{', '}'>
                    >
                > {};

        template <typename Rule>
        struct MeasureAttribParseAction : nothing<Rule> {};

        template <>
        struct MeasureAttribParseAction<StartOffset>
        {
            template<typename ActionInput>
            static void apply(const ActionInput &in,
                              std::optional<m32::ClefSignType>& clef,
                              std::optional<m32::KeySigType>& key,
                              std::optional<m32::TimeSigType>& time,
                              std::optional<m32::Offset>& start,
                              std::optional<m32::Offset>& length)
            { start = tinynote::parse_float_or_fraction<m32::Offset>(in.string()); }
        };

        template <>
        struct MeasureAttribParseAction<LengthDuration>
        {
            template<typename ActionInput>
            static void apply(const ActionInput &in,
                              std::optional<m32::ClefSignType>& clef,
                              std::optional<m32::KeySigType>& key,
                              std::optional<m32::TimeSigType>& time,
                              std::optional<m32::Offset>& start,
                              std::optional<m32::Offset>& length)
            { length = tinynote::parse_float_or_fraction<m32::Duration>(in.string()); }
        };

        template <>
        struct MeasureAttribParseAction<TimeValueNum>
        {
            template<typename ActionInput>
            static void apply(const ActionInput &in,
                              std::optional<m32::ClefSignType>& clef,
                              std::optional<m32::KeySigType>& key,
                              std::optional<m32::TimeSigType>& time,
                              std::optional<m32::Offset>& start,
                              std::optional<m32::Offset>& length)
            {
                if (!time.has_value()) time = std::make_pair(0, 0);
                time.value().first = std::stoi(in.string());
            }
        };

    template <>
    struct MeasureAttribParseAction<TimeValueDenom>
    {
        template<typename ActionInput>
        static void apply(const ActionInput &in,
                          std::optional<m32::ClefSignType>& clef,
                          std::optional<m32::KeySigType>& key,
                          std::optional<m32::TimeSigType>& time,
                          std::optional<m32::Offset>& start,
                          std::optional<m32::Offset>& length)
        {
            if (!time.has_value()) time = std::make_pair(0, 0);
            time.value().second = std::stoi(in.string());
        }
    };

        template <>
        struct MeasureAttribParseAction<TrebleStr>
        {
            template<typename ActionInput>
            static void apply(const ActionInput &in,
                              std::optional<m32::ClefSignType>& clef,
                              std::optional<m32::KeySigType>& key,
                              std::optional<m32::TimeSigType>& time,
                              std::optional<m32::Offset>& start,
                              std::optional<m32::Offset>& length)
            { clef = 'G'; }
        };

        template <>
        struct MeasureAttribParseAction<BassStr>
        {
            template<typename ActionInput>
            static void apply(const ActionInput &in,
                              std::optional<m32::ClefSignType>& clef,
                              std::optional<m32::KeySigType>& key,
                              std::optional<m32::TimeSigType>& time,
                              std::optional<m32::Offset>& start,
                              std::optional<m32::Offset>& length)
            { clef = 'F'; }
        };

        template <>
        struct MeasureAttribParseAction<AltoStr>
        {
            template<typename ActionInput>
            static void apply(const ActionInput &in,
                              std::optional<m32::ClefSignType>& clef,
                              std::optional<m32::KeySigType>& key,
                              std::optional<m32::TimeSigType>& time,
                              std::optional<m32::Offset>& start,
                              std::optional<m32::Offset>& length)
            { clef = 'C'; }
        };

        template <>
        struct MeasureAttribParseAction<KeyInt>
        {
            template<typename ActionInput>
            static void apply(const ActionInput &in,
                              std::optional<m32::ClefSignType>& clef,
                              std::optional<m32::KeySigType>& key,
                              std::optional<m32::TimeSigType>& time,
                              std::optional<m32::Offset>& start,
                              std::optional<m32::Offset>& length)
            { key = std::stol(in.string()); }
        };

        std::tuple<
                std::optional<m32::ClefSignType>,
                std::optional<m32::KeySigType>,
                std::optional<m32::TimeSigType>,
                std::optional<m32::Offset>,
                std::optional<m32::Duration>>
        parse_measure_attribs(std::string in);

        template <typename Rule>
        struct MeasureParseAction : nothing<Rule> {};

        template <>
        struct MeasureParseAction<BracketedListOf<MeasureAttrib, '[', ']'>>
        {
            template <typename ActionInput, typename MeasureT>
            static void apply(const ActionInput &in, MeasureT& measure)
            {
                auto [clef, key, time, start, length] = parse_measure_attribs(in.string());

                measure.mea_clef_sign = clef;
                measure.mea_key_sig = key;
                measure.mea_time_sig = time;
                typename decltype(length)::value_type measure_length = {0, 1};
                if (time.has_value())
                    measure_length = typename decltype(length)::value_type{ time.value().first * 4, time.value().second };
                measure.template set_length<KeepStart>(measure_length);
                if (start.has_value())
                    measure.template set_start<KeepLength>(start.value());
                if (length.has_value())
                    measure.template set_length<KeepStart>(length.value());
            }
        };

        template <>
        struct MeasureParseAction<MeasureSimpleNote>
        {
            template <typename ActionInput, typename MeasureT>
            static void apply(const ActionInput& in, MeasureT& measure)
            {
                auto snote = parse_simple_note(in.string());
                snote.template displace_start<KeepLength>(
                        measure.gnotes.empty()
                        ? 0
                        : measure.gnotes.back().get_end()
                        );
                measure.gnotes.push_back(snote);
            }
        };

        template <>
        struct MeasureParseAction<MeasureTuplet>
        {
            template <typename ActionInput, typename MeasureT>
            static void apply(const ActionInput& in, MeasureT& measure)
            {
                auto tuplet = parse_tuplet(in.string());
                tuplet.template displace_start<KeepLength>(
                        measure.gnotes.empty()
                        ? 0
                        : measure.gnotes.back().get_end()
                );
                measure.gnotes.push_back(tuplet);
            }
        };
    }

    inline std::tuple<
        std::optional<m32::ClefSignType>,
        std::optional<m32::KeySigType>,
        std::optional<m32::TimeSigType>,
        std::optional<m32::Offset>,
        std::optional<m32::Duration>>

    MeasureParserInternal::parse_measure_attribs(std::string in) {
        boost::erase_all(in, "\n");
        string_input inp (in, "parse_measure_attribs");
        std::optional<m32::ClefSignType> clef;
        std::optional<m32::KeySigType> key;
        std::optional<m32::TimeSigType> time;
        std::optional<m32::Offset> start;
        std::optional<m32::Duration> length;
        parse<
            BracketedListOf<MeasureParserInternal::MeasureAttrib, '[', ']'>,
            MeasureParserInternal::MeasureAttribParseAction
            >
        (inp, clef, key, time, start, length);
        return {clef, key, time, start, length};
    }

    template<typename OffsetT, typename DurationT, typename GNoteT = m32::GNote<OffsetT, DurationT>>
    inline m32::Measure<OffsetT, DurationT, GNoteT>
    parse_measure(std::string in) {
        boost::erase_all(in, "\n");
        string_input inp(in, "parse_measure");
        m32::Measure<OffsetT, DurationT, GNoteT> measure (0, 10, 0, {});
        parse<MeasureParserInternal::Measure, MeasureParserInternal::MeasureParseAction > (inp, measure);
        return measure;
    }
}

#endif //AIDA_MEASUREPARSER_H
