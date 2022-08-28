// Project AIDA
// Created by Long Duong on 12/21/21.
// Purpose: 
//

#include <catch2/catch.hpp>
#include <type_traits>
#include "common/Interval.h"

TEST_CASE("Interval(4, 10)") {
    auto interval = Interval<int>::from_end_points(4, 10);
    using IntervalType = std::decay_t<decltype(interval)>;

    SECTION("set-start") {
        SECTION("keep-length") {
            interval.set_start<KeepLength>(10);
            REQUIRE(interval.start == 10);
            REQUIRE(interval.length == 6);
            REQUIRE(interval.end == 16);
        }
        SECTION("keep-end") {
            interval.set_start<KeepEnd>(6);
            REQUIRE((interval.start == 6 && interval.length == 4 && interval.end == 10));
        }
    }

    SECTION("set-length") {
        SECTION("keep-start") {
            interval.set_length<KeepStart>(10);
            REQUIRE((interval.start == 4 && interval.length == 10 && interval.end == 14));
        }
        SECTION("keep-end") {
            interval.set_length<KeepEnd>(5);
            REQUIRE((interval.start == 5 && interval.length == 5 && interval.end == 10));
        }
    }

    SECTION("set-end") {
        SECTION("keep-start") {
            interval.set_end<KeepStart>(5);
            REQUIRE((interval.start == 4 && interval.length == 1 && interval.end == 5));
        }
        SECTION("keep-length") {
            interval.set_end<KeepLength>(5);
            REQUIRE((interval.start == -1 && interval.length == 6 && interval.end == 5));
        }
    }

    SECTION("displace-start") {
        SECTION("keep-end") {
            interval.displace_start<KeepEnd>(-5);
            // [-1, 10)
            REQUIRE((interval.start == -1 && interval.length == 11 && interval.end == 10));
        }
        SECTION("keep-length") {
            interval.displace_start<KeepLength>(-5);
            // [4, 10) -> [-1, 5)
            REQUIRE((interval.start == -1 && interval.length == 6 && interval.end == 5));
        }
    }

    SECTION("change-length-by") {
        SECTION("keep-start") {
            interval.change_length_by<KeepStart>(-5);
            REQUIRE((interval.start == 4 && interval.length == 1 && interval.end == 5));
        }
        SECTION("keep-end") {
            interval.change_length_by<KeepEnd>(2);
            REQUIRE((interval.start == 2 && interval.length == 8 && interval.end == 10));
        }
    }

    SECTION("scale-length-by") {
        SECTION("keep-start") {
            interval.scale_length_by<KeepStart>(3);
            REQUIRE((interval.start == 4 && interval.length == 18 && interval.end == 22));
        }
        SECTION("keep-end") {
            interval.scale_length_by<KeepEnd>(2);
            REQUIRE((interval.start == -2 && interval.length == 12 && interval.end == 10));
        }
    }

    SECTION("overlapping methods") {
        using OffsetType = decltype(interval.start);
        auto other = decltype(interval)::from_end_points(6, 15);
        auto other1 = decltype(interval)::from_end_points(10, 15);
        REQUIRE(interval.does_overlap_with(other));
        REQUIRE(!interval.does_overlap_with(other1));

        REQUIRE(interval.does_half_closed_interval_contain_offset(static_cast<OffsetType>(5)));
        REQUIRE(interval.does_half_closed_interval_contain_offset(static_cast<OffsetType>(4)));
        REQUIRE(!interval.does_half_closed_interval_contain_offset(static_cast<OffsetType>(10)));

        REQUIRE(interval.is_disjoint_with(other1));

        REQUIRE(!interval.does_swallow(other));
        REQUIRE(!interval.does_swallow(other1));

        auto other3 = decltype(interval)::from_end_points(5, 9);
        REQUIRE(interval.does_swallow(other3));
        REQUIRE(!interval.is_swallowed_by(other3));
        REQUIRE(other3.is_swallowed_by(interval));

        REQUIRE(interval.is_swallowed_by(interval));
        REQUIRE(!interval.is_strictly_swallowed_by(interval));
        REQUIRE(other3.is_strictly_swallowed_by(interval));
    }

    SECTION("intersection + merge") {
        auto other = decltype(interval)::from_end_points(6, 15);
        auto other1 = decltype(interval)::from_end_points(10, 15);

        auto intersect = interval.get_intersection_with(other); // [6, 10)
        REQUIRE((intersect.start == 6 && intersect.end == 10 && intersect.length == 4));

        interval.merge_with(other1);
        REQUIRE((interval.start == 4 && interval.end == 15 && interval.length == 11));
    }

}