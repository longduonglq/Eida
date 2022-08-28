// Project AIDA
// Created by Long Duong on 5/23/22.
// Purpose: 
//

#include <catch2/catch.hpp>
#include "m32/Measure.h"
#include "tinynote/MeasureParser.h"

using namespace tinynote;
TEST_CASE("m32::measure test") {
    using measure_t = m32::Measure<m32::Offset, m32::Duration>;

    SECTION("get-elts-acc-duration") {
        auto m = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[clef=treble, time=4/4, key=0] {
    C4:2, C4:1.5, rest:0.3
}
)");
        REQUIRE(m.get_elements_acc_duration() == m32::Duration{38, 10});
    }

    SECTION("is-rest-only") {
        auto m = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[clef=treble, time=4/4, key=0] {
    rest:0.3, rest:1
}
)");
        REQUIRE(m.is_rest_only());

        auto m1 = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[clef=treble, time=4/4, key=0] {
    rest:0.3, rest:1, Ch[C4, G5]:2
}
)");
        REQUIRE_FALSE(m1.is_rest_only());
    }

    SECTION("flat-const-iterator") {
        auto m = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[clef=treble, time=4/4, key=0] {
    C4:2, C4:1.5, rest:0.3
}
)");
        std::vector<typename decltype(m.gnotes)::value_type::SimpleNoteT> a = {
                parse_simple_note("C4:2"),
                parse_simple_note("C4:1.5[start=2]"),
                parse_simple_note("rest:0.3[start=3.5")
        };
        auto flat = m.flat_const_iterator();
        REQUIRE(std::equal(
                a.begin(), a.end(),
                flat.begin(),
                [](auto& a, auto& b) { return a.exact_eq(b); }
                ));
    }

    SECTION("set-start") {
        auto m = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[clef=treble, time=4/4, key=0] {
    C4:2, C4:1.5, rest:0.3
}
)");
        m.set_start<KeepLength>(m32::Offset(3, 2));
        std::vector<typename decltype(m.gnotes)::value_type::SimpleNoteT> a = {
                parse_simple_note("C4:2[start=1.5]"),
                parse_simple_note("C4:1.5[start=3.5]"),
                parse_simple_note("rest:0.3[start=5]")
        };
        auto flat = m.flat_const_iterator();
        REQUIRE(std::equal(
                a.begin(), a.end(),
                flat.begin(),
                [](auto& a, auto& b) { return a.exact_eq(b); }
        ));
    }

    SECTION("displace-start") {
        auto m = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[clef=treble, time=4/4, key=0] {
    C4:2, C4:1.5, rest:0.3
}
)");
        m.displace_start<KeepLength>(m32::Offset(3, 2));
        std::vector<typename decltype(m.gnotes)::value_type::SimpleNoteT> a = {
                parse_simple_note("C4:2[start=1.5]"),
                parse_simple_note("C4:1.5[start=3.5]"),
                parse_simple_note("rest:0.3[start=5]")
        };
        auto flat = m.flat_const_iterator();
        REQUIRE(std::equal(
                a.begin(), a.end(),
                flat.begin(),
                [](auto& a, auto& b) { return a.exact_eq(b); }
        ));
    }
}