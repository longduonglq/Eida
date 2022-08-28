//
// Created by dop on 12/27/21.
//

#include <catch2/catch.hpp>
#include <iostream>
#include <tao/pegtl.hpp>

#include <m32/Color.h>

#include "tinynote/SimpleNoteParser.h"

using namespace tao::pegtl;

TEST_CASE("without extra infos") {
    SECTION("1") {
        auto sn = tinynote::parse_simple_note("B4:1.5");
        REQUIRE((sn.pitches.size() == 1 &&
            sn.pitches[0].step == m32::Pitch::DiatonicStep::B &&
            sn.pitches[0].alter == m32::Pitch::Alter::No &&
            sn.pitches[0].octave == 4 &&
            sn.length == m32::Duration{15, 10}));
    }

    SECTION("2") {
        auto sn = tinynote::parse_simple_note("Gb3:2.5");
        REQUIRE((sn.pitches.size() == 1 &&
                 sn.pitches[0].step == m32::Pitch::DiatonicStep::G &&
                 sn.pitches[0].alter == m32::Pitch::Alter::Flat &&
                 sn.pitches[0].octave == 3 &&
                 sn.length == m32::Duration{25, 10}));
    }

    SECTION("3") {
        auto sn = tinynote::parse_simple_note("C#:1.5");
        REQUIRE((sn.pitches.size() == 1 &&
                 sn.pitches[0].step == m32::Pitch::DiatonicStep::C &&
                 sn.pitches[0].alter == m32::Pitch::Alter::Sharp &&
                 sn.pitches[0].octave == std::nullopt &&
                 sn.length == m32::Duration{15, 10}));
    }

    SECTION("fractional duration") {
        auto sn = tinynote::parse_simple_note("C#:1/5");
        REQUIRE((sn.pitches.size() == 1 &&
                 sn.pitches[0].step == m32::Pitch::DiatonicStep::C &&
                 sn.pitches[0].alter == m32::Pitch::Alter::Sharp &&
                 sn.pitches[0].octave == std::nullopt &&
                 sn.length == m32::Duration{1, 5}));
    }

    SECTION("duration with leading zeros") {
        auto sn = tinynote::parse_simple_note("C#:0.05");
        REQUIRE((sn.pitches.size() == 1 &&
                 sn.pitches[0].step == m32::Pitch::DiatonicStep::C &&
                 sn.pitches[0].alter == m32::Pitch::Alter::Sharp &&
                 sn.pitches[0].octave == std::nullopt &&
                 sn.length == m32::Duration{5, 100}));
    }
    SECTION("duration with leading zeros - 1") {
        auto sn = tinynote::parse_simple_note("C#:0.0625");
        REQUIRE((sn.pitches.size() == 1 &&
                 sn.pitches[0].step == m32::Pitch::DiatonicStep::C &&
                 sn.pitches[0].alter == m32::Pitch::Alter::Sharp &&
                 sn.pitches[0].octave == std::nullopt &&
                 sn.length == m32::Duration{625, 10000}));
    }
}

TEST_CASE("with extra infos") {
    SECTION("1") {
        auto sn = tinynote::parse_simple_note(R"(B4:1.5[tie_info=TieStart+TieEnd, lyrics=["asdf", "asdfa", "long duong"], color=#FF0000, start=2])");
        REQUIRE((sn.pitches.size() == 1 &&
                 sn.pitches[0].step == m32::Pitch::DiatonicStep::B &&
                 sn.pitches[0].alter == m32::Pitch::Alter::No &&
                 sn.pitches[0].octave == 4 &&
                 sn.length == m32::Duration{15, 10} &&
                 sn.start == m32::Offset{2, 1}));
        REQUIRE(((sn.tie_info & m32::TieStart) && (sn.tie_info & m32::TieEnd)));
        REQUIRE(sn.lyrics.size() == 3);
        REQUIRE(sn.lyrics[0].text == "asdf");
        REQUIRE(sn.lyrics[1].text == "asdfa");
        REQUIRE(sn.lyrics[2].text == "long duong");
        REQUIRE(sn.color.value() == m32::Color{255, 0, 0});
    }
}

TEST_CASE("chord with extra infos") {
    SECTION("1") {
        auto sn = tinynote::parse_simple_note(R"(Ch[B4, C#5, G]:1.5[tie_info=TieStart+TieEnd, lyrics=["asdf", "asdfa", "long duong"], color=#FF0000])");
        REQUIRE(sn.pitches.size() == 3);
        REQUIRE((sn.pitches[0].step == m32::Pitch::DiatonicStep::B &&
                 sn.pitches[0].alter == m32::Pitch::Alter::No &&
                 sn.pitches[0].octave == 4 &&
                 sn.length == m32::Duration{15, 10}));
        REQUIRE((sn.pitches[1].step == m32::Pitch::DiatonicStep::C &&
                 sn.pitches[1].alter == m32::Pitch::Alter::Sharp &&
                 sn.pitches[1].octave == 5 &&
                 sn.length == m32::Duration{15, 10}));
        REQUIRE((sn.pitches[2].step == m32::Pitch::DiatonicStep::G &&
                 sn.pitches[2].alter == m32::Pitch::Alter::No &&
                 sn.pitches[2].octave == std::nullopt &&
                 sn.length == m32::Duration{15, 10}));
        REQUIRE(((sn.tie_info & m32::TieStart) && (sn.tie_info & m32::TieEnd)));
        REQUIRE(sn.lyrics.size() == 3);
        REQUIRE(sn.lyrics[0].text == "asdf");
        REQUIRE(sn.lyrics[1].text == "asdfa");
        REQUIRE(sn.lyrics[2].text == "long duong");
        REQUIRE(sn.color.value() == m32::Color{255, 0, 0});
    }
    SECTION("2") {
        auto sn = tinynote::parse_simple_note(R"(Ch[B4, C#5, G]:1.257[lyrics=["asdf", "asdfa", "long duong"], color=#FF0000, start=1.5])");
        REQUIRE(sn.pitches.size() == 3);
        REQUIRE((sn.pitches[0].step == m32::Pitch::DiatonicStep::B &&
                 sn.pitches[0].alter == m32::Pitch::Alter::No &&
                 sn.pitches[0].octave == 4 &&
                 sn.length == m32::Duration{1257, 1000} &&
                 sn.start == m32::Duration{15, 10}));
        REQUIRE((sn.pitches[1].step == m32::Pitch::DiatonicStep::C &&
                 sn.pitches[1].alter == m32::Pitch::Alter::Sharp &&
                 sn.pitches[1].octave == 5 &&
                 sn.length == m32::Duration{1257, 1000} &&
                 sn.start == m32::Duration{15, 10}));
        REQUIRE((sn.pitches[2].step == m32::Pitch::DiatonicStep::G &&
                 sn.pitches[2].alter == m32::Pitch::Alter::No &&
                 sn.pitches[2].octave == std::nullopt &&
                 sn.length == m32::Duration{1257, 1000} &&
                 sn.start == m32::Duration{15, 10}));
        REQUIRE(sn.tie_info == m32::TieNeither);
        REQUIRE(sn.lyrics.size() == 3);
        REQUIRE(sn.lyrics[0].text == "asdf");
        REQUIRE(sn.lyrics[1].text == "asdfa");
        REQUIRE(sn.lyrics[2].text == "long duong");
        REQUIRE(sn.color.value() == m32::Color{255, 0, 0});
    }
}
