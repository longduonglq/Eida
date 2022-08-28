// Project AIDA
// Created by Long Duong on 5/22/22.
// Purpose: 
//

#include <catch2/catch.hpp>

#include "m32/SimpleNote.h"
#include "tinynote/SimpleNoteParser.h"

using namespace tinynote;
TEST_CASE("SimpleNote") {
    SECTION("is_* function") {
        SECTION("note") {
            auto nt = parse_simple_note("C#4:2");
            REQUIRE((nt.is_note() && !nt.is_chord() && !nt.is_rest()));
        }
        SECTION("chord") {
            auto ch = parse_simple_note("Ch[C4, E4, G5]:2.5");
            REQUIRE((!ch.is_note() && ch.is_chord() && !ch.is_rest()));
        }
        SECTION("rest") {
            auto rs = parse_simple_note("rest:3.75");
            REQUIRE((!rs.is_note() && !rs.is_chord() && rs.is_rest()));
        }
    }
    SECTION("split_at_offset") {
        SECTION("non-chord") {
            SECTION("test1") {
                auto nt = parse_simple_note("C#4:2");
                REQUIRE((nt.is_note() && !nt.is_chord() && !nt.is_rest()));
                SECTION("case 1") {
                    auto pair = nt.split_at_offset(1);
                    REQUIRE(pair.first.exact_eq(parse_simple_note("C#4:1[tie_info=TieStart]")));
                    REQUIRE(pair.second.exact_eq(parse_simple_note("C#4:1[start=1, tie_info=TieEnd]")));
                }

                SECTION("case 2") {
                    auto [first, second] = nt.split_at_offset({5, 10});
                    REQUIRE(first.exact_eq(parse_simple_note("C#4:0.5[tie_info=TieStart")));
                    REQUIRE(second.exact_eq(parse_simple_note("C#4:1.5[start=0.5, tie_info=TieEnd")));
                }
            }
        }
        SECTION("chord") {
            SECTION("test1") {
                auto ch = parse_simple_note("Ch[C4, E4, G5]:2.5");
                REQUIRE((!ch.is_note() && ch.is_chord() && !ch.is_rest()));

                SECTION("case 1- simple split") {
                    auto [first, second] = ch.split_at_offset(1);

                    REQUIRE(first.exact_eq(parse_simple_note("Ch[C4, E4, G5]:1[tie_info=TieStart")));
                    REQUIRE(second.exact_eq(parse_simple_note("Ch[C4, E4, G5]:1.5[start=1, tie_info=TieEnd")));
                }

                SECTION("case 2- float split") {
                    auto [first, second] = ch.split_at_offset({25, 100});

                    REQUIRE(first.exact_eq(parse_simple_note("Ch[C4, E4, G5]:0.25[tie_info=TieStart")));
                    REQUIRE(second.exact_eq(parse_simple_note("Ch[C4, E4, G5]:2.25[start=0.25, tie_info=TieEnd")));
                }
            }
        }
        SECTION("rest") {
            auto rs = parse_simple_note("rest:3.75");
            REQUIRE((!rs.is_note() && !rs.is_chord() && rs.is_rest()));

            auto [first, second] = rs.split_at_offset({13, 100});
            REQUIRE(first.exact_eq(parse_simple_note("rest:0.13[tie_info=TieStart")));
            REQUIRE(second.exact_eq(parse_simple_note("rest:3.62[start=0.13, tie_info=TieEnd")));
        }
    }

    SECTION("split and sever ties") {
        SECTION("chord") {
            SECTION("test1") {
                auto ch = parse_simple_note("Ch[C4, E4, G5]:2.5");
                REQUIRE((!ch.is_note() && ch.is_chord() && !ch.is_rest()));

                SECTION("case 1- simple split") {
                    auto [first, second] = ch.split_and_sever_ties_at_offset(1);

                    // std::cout << first << std::endl;
                    REQUIRE(first.exact_eq(parse_simple_note("Ch[C4, E4, G5]:1[tie_info=TieNeither]")));
                    REQUIRE(second.exact_eq(parse_simple_note("Ch[C4, E4, G5]:1.5[start=1, tie_info=TieNeither]")));
                }

                SECTION("case 2- float split") {
                    auto [first, second] = ch.split_and_sever_ties_at_offset({25, 100});

                    REQUIRE(first.exact_eq(parse_simple_note("Ch[C4, E4, G5]:0.25[tie_info=TieNeither]")));
                    REQUIRE(second.exact_eq(parse_simple_note("Ch[C4, E4, G5]:2.25[start=0.25, tie_info=TieNeither]")));
                }
            }
        }
        SECTION("rest") {
            auto rs = parse_simple_note("rest:3.75");
            REQUIRE((!rs.is_note() && !rs.is_chord() && rs.is_rest()));

            auto [first, second] = rs.split_and_sever_ties_at_offset({13, 100});
            REQUIRE(first.exact_eq(parse_simple_note("rest:0.13[tie_info=TieNeither]")));
            REQUIRE(second.exact_eq(parse_simple_note("rest:3.62[start=0.13, tie_info=TieNeither]")));
        }
    }

    SECTION("append-lyrics") {
        auto nt = parse_simple_note("C4:1[lyrics=[\"abc\", \"123\", \"xyz\"]]");
        REQUIRE(nt.lyrics.size() == 3);
        REQUIRE((nt.lyrics[0].number == 0 && nt.lyrics[0].text == "abc"));
        REQUIRE((nt.lyrics[1].number == 1 && nt.lyrics[1].text == "123"));
        REQUIRE((nt.lyrics[2].number == 2 && nt.lyrics[2].text == "xyz"));
    }

    SECTION("remove-lyric") {
        auto nt = parse_simple_note("C4:1[lyrics=[\"abc\", \"123\", \"xyz\"]]");
        nt.remove_lyric(nt.lyrics.begin() + 1);
        REQUIRE((nt.lyrics[0].number == 0 && nt.lyrics[0].text == "abc"));
        REQUIRE((nt.lyrics[1].number == 1 && nt.lyrics[1].text == "xyz"));
    }

    SECTION("ps-equal") {
        SECTION("1") {
            auto n1 = parse_simple_note("C4:1");
            auto n2 = parse_simple_note("B#3:1");
            // std::cout << n1 << n2 << std::endl;
            REQUIRE(n1.ps_eq(n2));
        }
        SECTION("2") {
            auto n1 = parse_simple_note("E4:1");
            auto n2 = parse_simple_note("Fb4:1");
            REQUIRE(n1.ps_eq(n2));
        }
        SECTION("3") {
            auto n1 = parse_simple_note("Gb4:1");
            auto n2 = parse_simple_note("F#4:1");
            REQUIRE(n1.ps_eq(n2));
        }
    }
}