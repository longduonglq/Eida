//
// Created by dmp on 1/19/22.
//

#include <catch2/catch.hpp>
#include <iostream>
#include <tao/pegtl.hpp>
#include <boost/rational.hpp>

#include "tinynote/Number.h"

using namespace tao::pegtl;

TEST_CASE("decimal") {
    using namespace tinynote;
    REQUIRE(parse_float_or_fraction<float>("3.14") == 3.14f);
    REQUIRE(parse_float_or_fraction<double>(".14") == 0.14);
    REQUIRE(parse_float_or_fraction<float>(".14") == 0.14f);
    REQUIRE(parse_float_or_fraction<float>("3.") == 3.0f);
    REQUIRE(parse_float_or_fraction<double>("3.") == 3.);
    REQUIRE(parse_float_or_fraction<float>("143.145") == 143.145f);

    REQUIRE(parse_float_or_fraction<float>("-143.145") == -143.145f);
    REQUIRE(parse_float_or_fraction<float>("-.14") == -0.14f);
    REQUIRE(parse_float_or_fraction<float>("-3.") == -3.0f);


    REQUIRE_THROWS(parse_float_or_fraction<float>("3/"));
    REQUIRE_THROWS(parse_float_or_fraction<float>("/5"));
}

TEST_CASE("fraction") {
    using namespace tinynote;
    using int_frac = boost::rational<int>;
    using long_frac = boost::rational<long>;
    REQUIRE(parse_float_or_fraction<int_frac>("3/5") == int_frac{3, 5});
    REQUIRE(parse_float_or_fraction<long_frac>("435345/324") == long_frac {435345, 324});
    REQUIRE_THROWS(parse_float_or_fraction<int_frac>("/43"));
    REQUIRE_THROWS(parse_float_or_fraction<long_frac>("24/"));
}