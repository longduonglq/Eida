//
// Created by dmp on 1/22/22.
//

#include <catch2/catch.hpp>
#include <iostream>
#include <tao/pegtl.hpp>
#include <boost/rational.hpp>

#include "tinynote/Length.h"

using namespace tao::pegtl;

TEST_CASE("Length") {
    using namespace tinynote;
    REQUIRE(parse_length<int>(".14") == boost::rational<int>{14, 100});
    REQUIRE(parse_length<long>("3.14") == boost::rational<long>{314, 100});
    REQUIRE(parse_length<int>("1/3") == boost::rational<int>{1, 3});
}