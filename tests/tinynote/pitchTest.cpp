//
// Created by dop on 12/27/21.
//

#include <catch2/catch.hpp>
#include <iostream>
#include <tao/pegtl.hpp>

#include "tinynote/PitchParser.h"

using namespace tao::pegtl;

TEST_CASE("step-parsing") {
    auto pitch = tinynote::parse_pitch("B#5");
    REQUIRE((pitch.step == m32::Pitch::DiatonicStep::B &&
        pitch.alter == m32::Pitch::Alter::Sharp &&
        pitch.octave == 5));
}
