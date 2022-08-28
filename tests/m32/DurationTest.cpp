//
// Created by dop on 12/26/21.
//

#include <catch2/catch.hpp>
#include <m32/Duration.h>
#include <cstring>

using namespace m32;

DurationName maximal_primitive_duration_name_lt(m32::Duration duration) {
    return duration_name_st<Supremum, Closed>(duration).value();
}
m32::Duration maximal_primitive_duration_lt(m32::Duration duration) {
    return duration_st<Supremum, Closed>(duration).value();
}

TEST_CASE("duration_str_from_name") {
    REQUIRE(std::strcmp(duration_str_from_name(DurationName::maxima), "maxima") == 0);
    REQUIRE(std::strcmp(duration_str_from_name(DurationName::dur32nd), "32nd") == 0);
    REQUIRE(std::strcmp(duration_str_from_name(DurationName::unspecified), "unspecified") == 0);
    REQUIRE(std::strcmp(duration_str_from_name(DurationName::dur1024th), "1024th") == 0);
    REQUIRE(std::strcmp(duration_str_from_name(DurationName::half), "half") == 0);
}

TEST_CASE("duration_name_from_str") {
    REQUIRE(duration_name_from_str("maxima") == DurationName::maxima);
    REQUIRE(duration_name_from_str("1024th") == DurationName::dur1024th);
    REQUIRE(duration_name_from_str("32nd") == DurationName::dur32nd);
    REQUIRE(duration_name_from_str("breve") == DurationName::breve);
    REQUIRE_THROWS(duration_name_from_str("sdfs"));
}

TEST_CASE("duration_from_duration_name") {
    REQUIRE(duration_from_duration_name(DurationName::maxima) == Duration{32, 1});
    REQUIRE(duration_from_duration_name(DurationName::dur1024th) == Duration{1, 256});
    REQUIRE_THROWS(duration_from_duration_name(DurationName::unspecified));
}

TEST_CASE("duration_name_from_duration") {
    REQUIRE(duration_name_from_duration(Duration{32, 1}) == DurationName::maxima);
    REQUIRE(duration_name_from_duration(Duration{1, 256}) == DurationName::dur1024th);
    REQUIRE_THROWS(duration_name_from_duration(Duration{1, 3}));
}

TEST_CASE("maximal_primitive_duration_name_lt") {
    REQUIRE(maximal_primitive_duration_name_lt({32, 1}) == duration_name_from_duration(m32::Duration{32, 1}));
    REQUIRE(maximal_primitive_duration_name_lt({33, 1}) == duration_name_from_duration(m32::Duration{32, 1}));
    REQUIRE(maximal_primitive_duration_name_lt({31, 1}) == duration_name_from_duration(m32::Duration{16, 1}));
    REQUIRE(maximal_primitive_duration_name_lt({17, 1}) == duration_name_from_duration(m32::Duration{16, 1}));
    REQUIRE(maximal_primitive_duration_name_lt({1, 127}) == duration_name_from_duration(m32::Duration{1, 128}));
    REQUIRE(maximal_primitive_duration_name_lt({1, 129}) == duration_name_from_duration(m32::Duration{1, 256}));
    REQUIRE(maximal_primitive_duration_name_lt({1, 256}) == duration_name_from_duration(m32::Duration{1, 256}));
}

TEST_CASE("maximal_primitive_duration_lt") {
    REQUIRE(maximal_primitive_duration_lt({32, 1}) == m32::Duration{32, 1});
    REQUIRE(maximal_primitive_duration_lt({33, 1}) == m32::Duration{32, 1});
    REQUIRE(maximal_primitive_duration_lt({31, 1}) == m32::Duration{16, 1});
    REQUIRE(maximal_primitive_duration_lt({17, 1}) == m32::Duration{16, 1});
    REQUIRE(maximal_primitive_duration_lt({1, 127}) == m32::Duration{1, 128});
    REQUIRE(maximal_primitive_duration_lt({1, 129}) == m32::Duration{1, 256});
    REQUIRE(maximal_primitive_duration_lt({1, 256}) == m32::Duration{1, 256});
}

TEST_CASE("decompose_duration_into_primitives") {
    SECTION("no dotted rhythm")
    {
        auto decomp  = decompose_duration_into_primitives(Duration{2, 1});
        REQUIRE(decomp.size() == 1);
        auto dur = decomp.at(0);
        REQUIRE(duration_name_from_duration(dur.first) == DurationName::half);
        REQUIRE(dur.second == 0);
    }
    SECTION("dotted rhythm")
    {
        auto decomp = decompose_duration_into_primitives(Duration{3, 1});
        REQUIRE(decomp.size() == 1);
        auto dur = decomp.at(0);
        REQUIRE(duration_name_from_duration(dur.first) == DurationName::half);
        REQUIRE(dur.second == 1);
    }
    SECTION("doubly dotted rhythm")
    {
        auto decomp = decompose_duration_into_primitives(Duration{350, 100});
        REQUIRE(decomp.size() == 1);
        auto dur = decomp.at(0);
        REQUIRE(duration_name_from_duration(dur.first) == DurationName::half);
        REQUIRE(dur.second == 2);
    }
    SECTION("undotable, with tie")
    {
        auto decomp = decompose_duration_into_primitives(Duration{250, 100});
        REQUIRE(decomp.size() == 2);

        REQUIRE(duration_name_from_duration(decomp.at(0).first) == DurationName::half);
        REQUIRE(decomp.at(0).second == 0);

        REQUIRE(duration_name_from_duration(decomp.at(1).first) == DurationName::eighth);
        REQUIRE(decomp.at(1).second == 0);
    }

    SECTION("undotable, with tie, 2nd note dotable")
    {
        auto decomp = decompose_duration_into_primitives(Duration{275, 100});
        REQUIRE(decomp.size() == 2);

        REQUIRE(duration_name_from_duration(decomp.at(0).first) == DurationName::half);
        REQUIRE(decomp.at(0).second == 0);

        REQUIRE(duration_name_from_duration(decomp.at(1).first) == DurationName::eighth);
        REQUIRE(decomp.at(1).second == 1);
    }

    SECTION("undotable, with tie, both notes dotable")
    {
        auto decomp = decompose_duration_into_primitives(Duration{3375, 1000});
        REQUIRE(decomp.size() == 2);

        REQUIRE(duration_name_from_duration(decomp.at(0).first) == DurationName::half);
        REQUIRE(decomp.at(0).second == 1);

        REQUIRE(duration_name_from_duration(decomp.at(1).first) == DurationName::dur16th);
        REQUIRE(decomp.at(1).second == 1);
    }

    SECTION("undotable, with tie, one note of decomp multi-dotable")
    {
        auto decomp = decompose_duration_into_primitives(Duration{36875, 10000});
        REQUIRE(decomp.size() == 2);

        REQUIRE(duration_name_from_duration(decomp.at(0).first) == DurationName::half);
        REQUIRE(decomp.at(0).second == 2);

        REQUIRE(duration_name_from_duration(decomp.at(1).first) == DurationName::dur32nd);
        REQUIRE(decomp.at(1).second == 1);
    }
}

TEST_CASE("duration_st") {
    SECTION("sup-open") {
        SECTION("endpoint wrapping test") {
            REQUIRE((duration_st<Supremum, Open>(Duration{33, 1}) == Duration{32, 1}));
            REQUIRE((duration_st<Supremum, Open>(Duration{32, 1}) == Duration{16, 1}));
            REQUIRE((duration_st<Supremum, Open>(Duration{1, 256}) == std::nullopt));
            REQUIRE((duration_st<Supremum, Open>(Duration{1, 257}) == std::nullopt));
        }
        SECTION("on point test") {
            REQUIRE((duration_st<Supremum, Open>(Duration{8, 1}) == Duration{4, 1}));
            REQUIRE((duration_st<Supremum, Open>(Duration{1, 16}) == Duration{1, 32}));
        }
        SECTION("off point test") {
            REQUIRE((duration_st<Supremum, Open>(Duration{3, 1}) == Duration{2, 1}));
            REQUIRE((duration_st<Supremum, Open>(Duration{1, 40}) == Duration{1, 64}));
        }
    }
    SECTION("sup-closed") {
        SECTION("endpoint wrapping test") {
            REQUIRE((duration_st<Supremum, Closed>(Duration{33, 1}) == Duration{32, 1}));
            REQUIRE((duration_st<Supremum, Closed>(Duration{32, 1}) == Duration{32, 1}));
            REQUIRE((duration_st<Supremum, Closed>(Duration{1, 256}) == Duration{1, 256}));
            REQUIRE((duration_st<Supremum, Closed>(Duration{1, 257}) == std::nullopt));
        }
        SECTION("on point test") {
            REQUIRE((duration_st<Supremum, Closed>(Duration{2, 1}) == Duration{2, 1}));
            REQUIRE((duration_st<Supremum, Closed>(Duration{1, 32}) == Duration{1, 32}));
        }
        SECTION("off point test") {
            REQUIRE((duration_st<Supremum, Closed>(Duration{15, 10}) == Duration{1, 1}));
            REQUIRE((duration_st<Supremum, Closed>(Duration{1, 70}) == Duration{1, 128}));
        }
    }
    SECTION("inf-open") {
        SECTION("endpoint wrapping test") {
            REQUIRE((duration_st<Infimum, Open>(Duration{33, 1}) == std::nullopt));
            REQUIRE((duration_st<Infimum, Open>(Duration{32, 1}) == std::nullopt));
            REQUIRE((duration_st<Infimum, Open>(Duration{1, 256}) == Duration{1, 128}));
            REQUIRE((duration_st<Infimum, Open>(Duration{1, 257}) == Duration{1, 256}));
        }

        SECTION("on point test") {
            REQUIRE((duration_st<Infimum, Open>(Duration{1, 1}) == Duration{2, 1}));
            REQUIRE((duration_st<Infimum, Open>(Duration{1, 128}) == Duration{1, 64}));
        }

        SECTION("off point test") {
            REQUIRE((duration_st<Infimum, Open>(Duration{9, 1}) == Duration{16, 1}));
            REQUIRE((duration_st<Infimum, Open>(Duration{1, 20}) == Duration{1, 16}));
        }
    }
    SECTION("inf-closec") {
        SECTION("endpoint wrapping test") {
            REQUIRE((duration_st<Infimum, Closed>(Duration{33, 1}) == std::nullopt));
            REQUIRE((duration_st<Infimum, Closed>(Duration{32, 1}) == Duration{32, 1}));
            REQUIRE((duration_st<Infimum, Closed>(Duration{1, 256}) == Duration{1, 256}));
            REQUIRE((duration_st<Infimum, Closed>(Duration{1, 257}) == Duration{1, 256}));
        }

        SECTION("on point test") {
            REQUIRE((duration_st<Infimum, Closed>(Duration{2, 1}) == Duration{2, 1}));
            REQUIRE((duration_st<Infimum, Closed>(Duration{1, 64}) == Duration{1, 64}));
        }

        SECTION("off point test") {
            REQUIRE((duration_st<Infimum, Closed>(Duration{20, 1}) == Duration{32, 1}));
            REQUIRE((duration_st<Infimum, Closed>(Duration{1, 200}) == Duration{1, 128}));
        }
    }
}

TEST_CASE("compute_dotted_length") {
    REQUIRE(compute_dotted_length(Duration{1, 4}, 2) == Duration{4375, 10000});
    REQUIRE(compute_dotted_length(Duration{1, 4}, 1) == Duration{375, 1000});
}