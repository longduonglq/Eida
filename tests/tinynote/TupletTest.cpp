//
// Created by dop on 12/27/21.
//

#include <catch2/catch.hpp>
#include <iostream>
#include <algorithm>
#include <tao/pegtl.hpp>

#include <m32/Color.h>

#include "tinynote/TupletParser.h"

using namespace tao::pegtl;

TEST_CASE("tuplet ") {
    SECTION("1") {
        auto sn = tinynote::parse_tuplet(R"(Tup@2!3[B4:1, C#5:1, G:1]:2)");
        REQUIRE(sn.notes.size() == 3);
        REQUIRE(sn.start == m32::Duration{0, 1});
        REQUIRE(sn.end == m32::Duration{2, 1});
        REQUIRE(sn.length == m32::Duration{2, 1});

        auto pitch_equal = [](m32::Pitch a, m32::Pitch b) {
            return (
                    a.step == b.step &&
                    a.alter == b.alter &&
                    a.octave == b.octave
                    );
        };
        auto note0 = tinynote::parse_simple_note("B4:2/3");
        REQUIRE((
                std::equal(sn.notes[0].pitches.begin(),
                           sn.notes[0].pitches.end(),
                           note0.pitches.begin(),
                           pitch_equal) &&
                sn.notes[0].length == note0.length));
        auto note1 = tinynote::parse_simple_note("C#5:2/3");
        REQUIRE((
                        std::equal(sn.notes[1].pitches.begin(),
                                   sn.notes[1].pitches.end(),
                                   note1.pitches.begin(),
                                   pitch_equal) &&
                        sn.notes[1].length == note1.length));
        auto note2 = tinynote::parse_simple_note("G:2/3");
        REQUIRE((
                        std::equal(sn.notes[2].pitches.begin(),
                                   sn.notes[2].pitches.end(),
                                   note2.pitches.begin(),
                                   pitch_equal) &&
                        sn.notes[2].length == note1.length));
        REQUIRE((sn.actual_number == 3 && sn.normal_number == 2));
    }
    SECTION("2") {
//        REQUIRE_THROWS(tinynote::parse_tuplet(R"(Tup@2[B4:1, C#5:1, G:1]:2[tie_info=TieStart+TieEnd, lyrics=["asdf", "asdfa", "long duong"], color=#FF0000])"));
    }
    SECTION("3") {
        auto tch = tinynote::parse_tuplet(R"(Tup@2!3[Ch[C4, E5, G5]:2, Ch[E3, G4, Bb5]:2, Ch[G5, B5, D5]:2]:2)");
        auto ch0 = tinynote::parse_simple_note("Ch[C4, E5, G5]:2/3");
        REQUIRE(ch0.exact_eq(tch.notes[0]));

        auto ch1 = tinynote::parse_simple_note("Ch[E3, G4, Bb5]:2/3");
        ch1.displace_start<KeepLength>(m32::Duration{2, 3});
        REQUIRE(ch1.exact_eq(tch.notes[1]));

        auto ch2 = tinynote::parse_simple_note("Ch[G5, B5, D5]:2/3");
        ch2.displace_start<KeepLength>(m32::Duration{4, 3});
        REQUIRE(ch2.exact_eq(tch.notes[2]));

        REQUIRE((tch.actual_number == 3 && tch.normal_number == 2));
    }

    SECTION("multi-digit normal number") {
        auto tup = tinynote::parse_tuplet("Tup@12!7[C4:1/16, C4:1/16, C4:1/16, C4:1/16, C4:1/16, C4:1/16, C4:1/16]:0.75");
        // tup.pack();
        REQUIRE((tup.normal_number == 12 && tup.actual_number == 7));
        REQUIRE(tup.notes.size() == 7);
    }
}


