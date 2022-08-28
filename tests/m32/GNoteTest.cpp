//
// Created by dmp on 1/22/22.
//

#include <catch2/catch.hpp>
#include <vector>
#include <variant>

#include "tinynote/SimpleNoteParser.h"
#include "tinynote/TupletParser.h"

#include "m32/SimpleNote.h"
#include "m32/Tuplet.h"
#include "m32/Duration.h"
#include "m32/GNote.h"

using namespace std;
using namespace m32;

TEST_CASE("gnote") {
    SECTION("simple_note") {
        auto gn = GNote<m32::Duration, m32::Duration>(
                std::in_place_type<SimpleNote<m32::Duration, m32::Duration>>,
                tinynote::parse_simple_note("C#4:2.5"));
        REQUIRE(std::holds_alternative<SimpleNote<m32::Duration, m32::Duration>>(gn));
        REQUIRE(std::get<SimpleNote<Duration, Duration>>(gn).pitches.size() == 1);
        REQUIRE(std::get<SimpleNote<Duration, Duration>>(gn).pitches[0].exact_eq(tinynote::parse_pitch("C#4")));
        REQUIRE(std::get<SimpleNote<Duration, Duration>>(gn).length == Duration{25, 10});

        SECTION("gnote get functions") {
            REQUIRE(gn.get_start() == Duration{0, 1});
            REQUIRE(gn.get_end() == Duration{25, 10});
            REQUIRE(gn.get_length() == Duration{25, 10});
        }

        SECTION("flatten_if_tuplet") {
//            auto flat = gn.flatten_if_tuplet();
//            REQUIRE(flat.size() == 1);
//            REQUIRE(flat[0].pitches[0].exact_eq(tinynote::parse_pitch("C#4")));
//            REQUIRE(flat[0].pitches.size() == 1);
//            REQUIRE(flat[0].length == Duration{25, 10});
        }
    }
    SECTION("tuplet") {
        auto t = tinynote::parse_tuplet(R"(Tup@2[B4:1, C#5:1, G:1]:2)");
        auto gn = GNote<m32::Duration, m32::Duration>(
                std::in_place_type<Tuplet<m32::Duration, m32::Duration>>,
                tinynote::parse_tuplet(R"(Tup@2[B4:1, C#5:1, G:1]:2)"));
        REQUIRE(std::holds_alternative<Tuplet<m32::Duration, m32::Duration>>(gn));

        auto sn = std::get<Tuplet<Duration, Duration>>(gn);
        REQUIRE(sn.notes.size() == 3);
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
                        sn.notes[1].length == note0.length));
        auto note2 = tinynote::parse_simple_note("G:2/3");
        REQUIRE((
                        std::equal(sn.notes[2].pitches.begin(),
                                   sn.notes[2].pitches.end(),
                                   note2.pitches.begin(),
                                   pitch_equal) &&
                        sn.notes[2].length == note0.length));
        REQUIRE((sn.actual_number == 3 && sn.normal_number == 2));

        SECTION("gnote get functions") {
            REQUIRE(gn.get_start() == Duration{0, 1});
            REQUIRE(gn.get_end() == Duration{20, 10});
            REQUIRE(gn.get_length() == Duration{20, 10});
        }

        SECTION("flatten_if_tuplet") {
            // TODO: application-specific, do later
//            auto flat = gn.flatten_if_tuplet();
//            REQUIRE(flat.size() == 3);
//            auto note0 = tinynote::parse_simple_note("B4:1");
//            REQUIRE((
//                            std::equal(flat[0].pitches.begin(),
//                                       flat[0].pitches.end(),
//                                       note0.pitches.begin(),
//                                       pitch_equal) &&
//                                    flat[0].length == note0.length));
//            auto note1 = tinynote::parse_simple_note("C#5:1");
//            REQUIRE((
//                            std::equal(flat[1].pitches.begin(),
//                                       flat[1].pitches.end(),
//                                       note1.pitches.begin(),
//                                       pitch_equal) &&
//                                    flat[1].length == note0.length));
//            auto note2 = tinynote::parse_simple_note("G:1");
//            REQUIRE((
//                            std::equal(flat[2].pitches.begin(),
//                                       flat[2].pitches.end(),
//                                       note2.pitches.begin(),
//                                       pitch_equal) &&
//                                    flat[2].length == note0.length));
        }

        SECTION("split_at_offset") {
        }
    }
}