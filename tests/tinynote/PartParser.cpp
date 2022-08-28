// Project AIDA
// Created by Long Duong on May21, 2022.
// Purpose: test part parser
//

#include <catch2/catch.hpp>
#include <vector>
#include <variant>
#include <iostream>
#include <m32/GNote.h>

#include "tinynote/PartParser.h"

TEST_CASE("measured part parser") {
    auto mpart = tinynote::parse_measured_part(R"(
MeasuredPart[time=4/4, clef=treble, key=4] {
    Mea [time=4/4, clef=treble, key=4] {
        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
    },
    Mea {
        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
    }
}
)");
    REQUIRE(mpart.measures.size() == 2);
    REQUIRE((mpart.clef_sign == 'G'
             && mpart.key_sig == 4
             && mpart.time_sig == m32::TimeSigType{4, 4}));
    {
        auto& measure = mpart.measures[0];
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
                        }
                )
        );

        REQUIRE(measure.gnotes.size() == 4);
    }
    {
        auto& measure = mpart.measures[1];
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
                        }
                )
        );

        REQUIRE(measure.gnotes.size() == 4);
    }
}

TEST_CASE("part parser") {
    using gnote_t = m32::GNote<m32::Offset, m32::Duration>;

    auto part = tinynote::parse_part<m32::Offset, m32::Duration, gnote_t>(R"(
Part[time=4/4, clef=bass, key=4] {
        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2,

        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
}
)");
    REQUIRE((part.clef_sign == 'F'
             && part.key_sig == 4
             && part.time_sig == m32::TimeSigType{4, 4}));

    {
        auto note0 = tinynote::parse_simple_note("C#4:2");
        REQUIRE(std::get<decltype(note0)>(part.gnotes[0]).exact_eq(note0));

        auto note1 = tinynote::parse_simple_note("Gb5:2");
        note1.displace_start<KeepLength>(2);
        REQUIRE(std::get<decltype(note1)>(part.gnotes[1]).exact_eq(note1));

        auto note2 = tinynote::parse_simple_note("Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd]");
        note2.displace_start<KeepLength>(4);
        REQUIRE(std::get<decltype(note2)>(part.gnotes[2]).exact_eq(note2));

        auto note3 = tinynote::parse_tuplet("Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2");
        note3.displace_start<KeepLength>(8);
        REQUIRE(
                std::equal(
                        std::get<decltype(note3)>(part.gnotes[3]).notes.begin(),
                        std::get<decltype(note3)>(part.gnotes[3]).notes.end(),
                        note3.notes.begin(),
                        [](auto& a, auto& b)
                        {
                            return a.exact_eq(b);
                        }
                )
        );

        REQUIRE(part.gnotes.size() == 8);
    }
    {
        auto note0 = tinynote::parse_simple_note("C#4:2");
        note0.displace_start<KeepLength>(10);
        REQUIRE(std::get<decltype(note0)>(part.gnotes[4]).exact_eq(note0));

        auto note1 = tinynote::parse_simple_note("Gb5:2");
        note1.displace_start<KeepLength>(12);
        REQUIRE(std::get<decltype(note1)>(part.gnotes[5]).exact_eq(note1));

        auto note2 = tinynote::parse_simple_note("Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd]");
        note2.displace_start<KeepLength>(14);
        REQUIRE(std::get<decltype(note2)>(part.gnotes[6]).exact_eq(note2));

        auto note3 = tinynote::parse_tuplet("Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2");
        note3.displace_start<KeepLength>(18);
        REQUIRE(
                std::equal(
                        std::get<decltype(note3)>(part.gnotes[7]).notes.begin(),
                        std::get<decltype(note3)>(part.gnotes[7]).notes.end(),
                        note3.notes.begin(),
                        [](auto& a, auto& b)
                        {
                            return a.exact_eq(b);
                        }
                )
        );

        REQUIRE(part.gnotes.size() == 8);
    }
}