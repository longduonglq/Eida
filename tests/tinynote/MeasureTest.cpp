//
// Created by dmp on 1/23/22.
//

#include <catch2/catch.hpp>
#include <vector>
#include <variant>

#include "tinynote/SimpleNoteParser.h"
#include "tinynote/TupletParser.h"
#include "tinynote/MeasureParser.h"

#include "m32/SimpleNote.h"
#include "m32/Tuplet.h"
#include "m32/Duration.h"
#include "m32/GNote.h"

using namespace std;
using namespace m32;

TEST_CASE("measure") {
    using GNoteT = m32::GNote<m32::Duration, m32::Duration>;
    auto measure = tinynote::parse_measure<m32::Duration, m32::Duration, GNoteT>
    (R"(Mea [time=4/4, clef=treble, key=4]
    {
        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
    })");


    REQUIRE((measure.mea_clef_sign == 'G'
        && measure.mea_key_sig == 4
        && measure.mea_time_sig == TimeSigType{4, 4}));

    using gnote_t = GNote<Offset, Duration>;
    auto note0 = tinynote::parse_simple_note("C#4:2");
    REQUIRE(std::get<decltype(note0)>(measure.gnotes[0]).exact_eq(note0));

    auto note1 = tinynote::parse_simple_note("Gb5:2");
    note1.displace_start<KeepLength>(2);
    REQUIRE(std::get<decltype(note1)>(measure.gnotes[1]).exact_eq(note1));

    auto note2 = tinynote::parse_simple_note("Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd]");
    note2.displace_start<KeepLength>(4);
    REQUIRE(std::get<decltype(note2)>(measure.gnotes[2]).exact_eq(note2));

    auto note3 = tinynote::parse_tuplet("Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2");
    note3.displace_start<KeepLength>(8);
    REQUIRE(
            std::equal(
                    std::get<decltype(note3)>(measure.gnotes[3]).notes.begin(),
                    std::get<decltype(note3)>(measure.gnotes[3]).notes.end(),
                    note3.notes.begin(),
                    [](auto& a, auto& b)
                    {
                        return a.exact_eq(b);
                    })
            );

    REQUIRE(measure.gnotes.size() == 4);
}

TEST_CASE("init start and length") {
    SECTION("1") {
        auto m = tinynote::parse_measure<m32::Offset, m32::Duration>(R"(Mea[clef=treble, time=4/4, key=0, start=5, length=1]{C4:2, C4:1.5, rest:0.3})");
        REQUIRE(m.start == m32::Duration{5, 1});
        REQUIRE(m.length == m32::Duration{1, 1});
    }
    SECTION("2") {
        auto m = tinynote::parse_measure<m32::Offset, m32::Duration>(R"(Mea[clef=treble, time=4/4, key=0, start=5.3, length=1/9]{C4:2, C4:1.5, rest:0.3})");

        REQUIRE(m.start == m32::Duration{53, 10});
        REQUIRE(m.length == m32::Duration{1, 9});
    }
}