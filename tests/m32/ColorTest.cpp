//
// Created by dop on 12/25/21.
//

#include <catch2/catch.hpp>
#include "m32/Color.h"

using namespace m32;
TEST_CASE("HEX -> Color object") {
    SECTION("1") {
        auto color = Color::from_hex_rgb("#FF00FF");
        REQUIRE((color.red == 255 && color.green == 0 && color.blue == 255));

        auto color1 = Color::from_hex_rgb("#ABFF00FF");
        REQUIRE((color1.red == 255 && color1.green == 0 && color1.blue == 255));
    }
}

TEST_CASE("Color object -> HEX") {
    SECTION("1") {
        Color color {255, 255, 0};
        REQUIRE(color.to_hex_rgb() == "#FFFF00");
    }
}

