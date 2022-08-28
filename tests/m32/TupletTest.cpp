// Project AIDA
// Created by Long Duong on 5/22/22.
// Purpose: 
//

#include <catch2/catch.hpp>
#include <numeric>
#include <algorithm>

#include "m32/Tuplet.h"
#include "m32/Duration.h"
#include "tinynote/TupletParser.h"
#include "tinynote/SimpleNoteParser.h"
#include "show.h"

using namespace tinynote;

TEST_CASE("tuplet") {
    SECTION("pack()") {
        SECTION("test 1") {
            auto tup = parse_tuplet("Tup@2[C4:1, E4:1, G5:1]:2");
            tup.pack();
            REQUIRE(tup.notes[0].exact_eq(parse_simple_note("C4:2/3")));
            REQUIRE(tup.notes[1].exact_eq(parse_simple_note("E4:2/3[start=2/3]")));
            REQUIRE(tup.notes[2].exact_eq(parse_simple_note("G5:2/3[start=4/3]")));
        }
        SECTION("test 2") {
            auto tup = parse_tuplet("Tup@2[C4:1, E4:1.5, G5:1]:2");
            tup.pack();
            REQUIRE(tup.notes[0].exact_eq(parse_simple_note("C4:4/7")));
            REQUIRE(tup.notes[1].exact_eq(parse_simple_note("E4:6/7[start=4/7]")));
            REQUIRE(tup.notes[2].exact_eq(parse_simple_note("G5:4/7[start=10/7]")));
        }
        SECTION("test 3") {
            auto tup = parse_tuplet("Tup@2!3[C4:2, E4:1]:2");
            tup.pack();
            REQUIRE(tup.notes[0].exact_eq(parse_simple_note("C4:4/3")));
            REQUIRE(tup.notes[1].exact_eq(parse_simple_note("E4:2/3[start=4/3]")));
        }
    }

    SECTION("set_start") {
        auto tup = parse_tuplet("Tup@2[C4:1, E4:1, G5:1]:2");
        auto tup1 = parse_tuplet("Tup@2[C4:1, E4:1, G5:1]:2");

        SECTION("test 1") {
            tup1.set_start<KeepLength>(1);
            REQUIRE((tup1.actual_number == tup.actual_number && tup1.normal_number == tup.normal_number));
            REQUIRE((
                    tup1.start == tup.start + 1 &&
                    tup1.end == tup.end + 1 &&
                    tup1.length == tup.length));
            auto& notes0 = tup.notes;
            auto& notes1 = tup1.notes;
            REQUIRE((
                    notes1[0].start == notes0[0].start + 1&&
                    notes1[0].end == notes0[0].end + 1 &&
                    notes1[0].length == notes0[0].length &&

                    notes1[1].start == notes0[1].start + 1&&
                    notes1[1].end == notes0[1].end + 1 &&
                    notes1[1].length == notes0[1].length &&

                    notes1[2].start == notes0[2].start + 1&&
                    notes1[2].end == notes0[2].end + 1 &&
                    notes1[2].length == notes0[2].length
                    ));

            auto tup1_std = tinynote::parse_tuplet("Tup@2[C4:1, E4:1, G5:1]:2[start=1]");
            REQUIRE(tup1_std.exact_eq(tup1));
        }

        SECTION("test 2") {
            tup1.set_start<KeepLength>(m32::Duration{25, 100});
            REQUIRE((tup1.actual_number == tup.actual_number && tup1.normal_number == tup.normal_number));
            REQUIRE((
                            tup1.start == tup.start + m32::Duration{25, 100} &&
                            tup1.end == tup.end + m32::Duration{25, 100} &&
                            tup1.length == tup.length));
            auto& notes0 = tup.notes;
            auto& notes1 = tup1.notes;
            REQUIRE((
                            notes1[0].start == notes0[0].start + m32::Duration{25, 100} &&
                            notes1[0].end == notes0[0].end + m32::Duration{25, 100} &&
                            notes1[0].length == notes0[0].length &&

                            notes1[1].start == notes0[1].start + m32::Duration{25, 100} &&
                            notes1[1].end == notes0[1].end + m32::Duration{25, 100} &&
                            notes1[1].length == notes0[1].length &&

                            notes1[2].start == notes0[2].start + m32::Duration{25, 100} &&
                            notes1[2].end == notes0[2].end + m32::Duration{25, 100} &&
                            notes1[2].length == notes0[2].length
                    ));
        }
    }

    SECTION("displace_start") {
        auto tup = parse_tuplet("Tup@2[C4:1, E4:1, G5:1]:2");
        auto tup1 = parse_tuplet("Tup@2[C4:1, E4:1, G5:1]:2");

        tup1.displace_start<KeepLength>(1);
        REQUIRE((tup1.actual_number == tup.actual_number && tup1.normal_number == tup.normal_number));
        REQUIRE((
                        tup1.start == tup.start + 1 &&
                        tup1.end == tup.end + 1 &&
                        tup1.length == tup.length));
        auto& notes0 = tup.notes;
        auto& notes1 = tup1.notes;
        REQUIRE((
                        notes1[0].start == notes0[0].start + 1&&
                        notes1[0].end == notes0[0].end + 1 &&
                        notes1[0].length == notes0[0].length &&

                        notes1[1].start == notes0[1].start + 1&&
                        notes1[1].end == notes0[1].end + 1 &&
                        notes1[1].length == notes0[1].length &&

                        notes1[2].start == notes0[2].start + 1&&
                        notes1[2].end == notes0[2].end + 1 &&
                        notes1[2].length == notes0[2].length
                ));
    }

    SECTION("change-length-by") {
        SECTION("test 1 - check that [change-length-by] and [pack] are commutative") {
            auto tup = parse_tuplet("Tup@2[C4:1, E4:1, G5:1]:2");
            auto tup1 = parse_tuplet("Tup@2[C4:1, E4:1, G5:1]:2");

            tup.change_length_by(m32::Duration{-1, 2});
            tup.pack();

            tup1.pack();
            tup1.change_length_by(m32::Duration{-1, 2});

            REQUIRE(tup1.exact_eq(tup));
        }

        SECTION("test 2") {
            auto tup = parse_tuplet("Tup@2[C4:1, E4:1, G5:1]:2");
            tup.change_length_by(m32::Duration{+3, 4});
            tup.pack();
            REQUIRE(std::all_of(tup.notes.begin(), tup.notes.end(),
                                [&](auto& nt){ return nt.length == tup.notes[0].length ;}));
            REQUIRE(tup.notes.front().length / tup.length == m32::Duration{1, 3});
        }

        SECTION("test 3") {
            auto tup = parse_tuplet("Tup@2[C4:1, E4:1.5, G5:1]:2");
            tup.change_length_by(m32::Duration{+3, 4});
            tup.pack();
            REQUIRE(tup.notes[1].length / tup.notes[0].length == m32::Duration{3, 2});
            REQUIRE(tup.notes[1].length / tup.notes[2].length == m32::Duration{3, 2});
            REQUIRE(tup.notes[0].length / tup.notes[2].length == m32::Duration{1, 1});
            REQUIRE(std::reduce(tup.notes.begin(), tup.notes.end(), m32::Duration{0},
                                [](auto& a, const auto& b){ return a + b.length; }) == tup.length);
        }
    }

    SECTION("tuplet inner note type test") {
        REQUIRE((m32::duration_st<m32::Infimum, m32::Closed>(m32::Duration{2, 3}) == m32::Duration{1, 1}));
        REQUIRE((m32::duration_st<m32::Infimum, m32::Closed>(m32::Duration{2, 4}) == m32::Duration{1, 2}));
        REQUIRE((m32::duration_st<m32::Infimum, m32::Closed>(m32::Duration{2, 5}) == m32::Duration{1, 2}));
        REQUIRE((m32::duration_st<m32::Infimum, m32::Closed>(m32::Duration{2, 6}) == m32::Duration{1, 2}));
        REQUIRE((m32::duration_st<m32::Infimum, m32::Closed>(m32::Duration{2, 7}) == m32::Duration{1, 2}));
        REQUIRE((m32::duration_st<m32::Infimum, m32::Closed>(m32::Duration{2, 8}) == m32::Duration{1, 4}));
        REQUIRE((m32::duration_st<m32::Infimum, m32::Closed>(m32::Duration{2, 9}) == m32::Duration{1, 4}));

        REQUIRE((m32::duration_st<m32::Infimum, m32::Closed>(m32::Duration{1, 9}) == m32::Duration{1, 8}));
    }

    SECTION("flatten") {
        SECTION("test 1") {
            auto tup = parse_tuplet("Tup@2!3[C4:2, E4:1]:2");
            tup.pack();
            auto flat = tup.flatten();
            REQUIRE(flat.size() == 2);
            REQUIRE(flat[0].exact_eq(parse_simple_note("C4:2")));
            REQUIRE(flat[1].exact_eq(parse_simple_note("E4:1[start=2]")));
        }
        SECTION("test 1") {
            auto tup = parse_tuplet("Tup@2!3[C4:0.5, E4:1.5, G5:1]:2");
            tup.pack();
            auto flat = tup.flatten();
            REQUIRE(flat.size() == 3);
            REQUIRE(flat[0].exact_eq(parse_simple_note("C4:0.5")));
            REQUIRE(flat[1].exact_eq(parse_simple_note("E4:1.5[start=0.5]")));
            REQUIRE(flat[2].exact_eq(parse_simple_note("G5:1[start=2]")));
        }
    }

    SECTION("packing is inner-length agnostic") {
        auto tup = parse_tuplet("Tup@12!7[C4:3/16, C4:1/16, C4:1/16, C4:2/16]:0.75");
        tup.pack();

        auto tup1 = parse_tuplet("Tup@12!7[C4:3, C4:1, C4:1, C4:2]:0.75");
        tup1.pack();
        REQUIRE(tup.exact_eq(tup1));
    }

    SECTION("split-at-offset") {
        SECTION("test 1") {
            auto tup = parse_tuplet("Tup@2!3[C4:0.5, E4:0.5, G4:0.5]:1");
            tup.pack();
            auto pair = tup.split_at_offset(m32::Duration{1, 2});
            auto [left, right] = pair;

            auto left_tup = parse_tuplet("Tup@2!3[C4:0.5, E4:0.25[tie_info=TieStart]]:0.5");
            left_tup.pack();
            auto right_tup = parse_tuplet("Tup@2!3[E4:0.25[tie_info=TieEnd], G4:0.5]:0.5[start=0.5]");
            right_tup.pack();
            REQUIRE(left.exact_eq(left_tup));
            REQUIRE(right.exact_eq(right_tup));
        }
        SECTION("test 2") {
            auto tup = parse_tuplet("Tup@2!3[C4:0.5, E4:0.5, G4:0.5]:1");
            tup.pack();
            auto pair = tup.split_at_offset(m32::Duration{3, 4});
            auto [left, right] = pair;

            auto left_tup = parse_tuplet("Tup@6!9[C4:0.5, E4:0.5, G4:0.125[tie_info=TieStart]]:0.75");
            left_tup.pack();
            auto right_tup = parse_simple_note("G4:0.25[tie_info=TieEnd, start=0.75]");
            REQUIRE(left.exact_eq(left_tup));
            REQUIRE(right.exact_eq(right_tup));
        }
        SECTION("test 3") {
            auto tup = parse_tuplet("Tup@2!3[C4:0.5, E4:0.5, G4:0.5]:1");
            tup.pack();
            auto pair = tup.split_at_offset(m32::Duration{1, 4});
            auto [left, right] = pair;

            auto left_nt = parse_simple_note("C4:0.25[tie_info=TieStart]");
            auto right_tup = parse_tuplet("Tup@6!9[C4:0.125[tie_info=TieEnd], E4:0.5, G4:0.5]:0.75[start=0.25]");
            right_tup.pack();

            REQUIRE(right.exact_eq(right_tup));
            REQUIRE(left.exact_eq(left_nt));
        }

        SECTION("test 4") {
            auto tup = parse_tuplet("Tup@2!3[C4:0.5, E4:0.5, G4:0.5]:1[start=2]");
            tup.pack();
            REQUIRE_THROWS(tup.split_at_offset(m32::Duration{1, 4}));
            auto pair = tup.split_at_offset(m32::Duration{1, 4} + 2);
            auto [left, right] = pair;

            auto left_nt = parse_simple_note("C4:0.25[tie_info=TieStart, start=2]");
            auto right_tup = parse_tuplet("Tup@6!9[C4:0.125[tie_info=TieEnd], E4:0.5, G4:0.5]:0.75[start=2.25]");
            right_tup.pack();

            REQUIRE(right.exact_eq(right_tup));
            REQUIRE(left.exact_eq(left_nt));
        }

        SECTION("test 5") {
            auto tup = parse_tuplet("Tup@2!3[C4:0.5, E4:0.5, G4:0.5]:1[start=2]");
            tup.pack();
            REQUIRE_THROWS(tup.split_at_offset(m32::Duration{3, 4}));
            auto pair = tup.split_at_offset(m32::Duration{3, 4} + 2);
            auto [left, right] = pair;

            auto left_tup = parse_tuplet("Tup@6!9[C4:0.5, E4:0.5, G4:0.125[tie_info=TieStart]]:0.75[start=2]");
            auto right_nt = parse_simple_note("G4:0.25[tie_info=TieEnd, start=2.75]");
            left_tup.pack();

            REQUIRE(right.exact_eq(right_nt));
            REQUIRE(left.exact_eq(left_tup));
        }

        SECTION("makes sure split leaves normal_number untouched") {
            auto tup1 = parse_tuplet("Tup@0!7[C4:3, C4:2, C4:2]:1");
            tup1.pack();

            auto tup2 = parse_tuplet("Tup@12!7[C4:3, C4:2, C4:2]:1");
            tup2.pack();

            tup1.normal_number = 12; // the only difference is tolerable
            REQUIRE(tup1.exact_eq(tup2));
        }

        SECTION("test 6") {
            auto tup = parse_tuplet("Tup@4!5[C4:0.25, C4:0.25, C4:0.25, C4:0.25, C4:0.25]:1");
            tup.pack();
            auto pair = tup.split_at_offset(m32::Duration{1, 4});
            auto [left, right] = pair;

            // std::cout << tup << std::endl;
            // std::cout << left << right << std::endl;
            auto left_tup = parse_tuplet("Tup@4!5[C4:0.2, C4:0.05[tie_info=TieStart]]:0.25");
            auto right_tup = parse_tuplet("Tup@12!15[C4:0.15[tie_info=TieEnd], C4:0.2, C4:0.2, C4:0.2]:0.75[start=0.25]");
            left_tup.pack();
            right_tup.pack();
            // std::cout << left_tup << right_tup << std::endl;

            // m32::show::show({left_tup, right_tup});
            REQUIRE(right.exact_eq(right_tup));
            REQUIRE(left.exact_eq(left_tup));
        }
    }
    SECTION("cut and sever ties") {
        SECTION("test 3") {
            auto tup = parse_tuplet("Tup@2!3[C4:0.5, E4:0.5, G4:0.5]:1");
            tup.pack();
            auto pair = tup.split_and_sever_ties_at_offset(m32::Duration{1, 4});
            auto [left, right] = pair;

            auto left_nt = parse_simple_note("C4:0.25[tie_info=TieNeither]");
            auto right_tup = parse_tuplet("Tup@6!9[C4:0.125[tie_info=TieNeither], E4:0.5, G4:0.5]:0.75[start=0.25]");
            right_tup.pack();

            // std::cout << right << right_tup << std::endl;
            REQUIRE(right.exact_eq(right_tup));
            REQUIRE(left.exact_eq(left_nt));
        }

        SECTION("test 4") {
            auto tup = parse_tuplet("Tup@2!3[C4:0.5, E4:0.5, G4:0.5]:1[start=2]");
            tup.pack();
            REQUIRE_THROWS(tup.split_and_sever_ties_at_offset(m32::Duration{1, 4}));
            auto pair = tup.split_and_sever_ties_at_offset(m32::Duration{1, 4} + 2);
            auto [left, right] = pair;

            auto left_nt = parse_simple_note("C4:0.25[tie_info=TieNeither, start=2]");
            auto right_tup = parse_tuplet("Tup@6!9[C4:0.125[tie_info=TieNeither], E4:0.5, G4:0.5]:0.75[start=2.25]");
            right_tup.pack();

            REQUIRE(right.exact_eq(right_tup));
            REQUIRE(left.exact_eq(left_nt));
        }
    }
}
