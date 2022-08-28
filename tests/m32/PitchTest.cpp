//
// Created by dop on 12/26/21.
//

#include <catch2/catch.hpp>
#include <vector>
#include "m32/Pitch.h"

#define MAKE_PITCH(step, octave, alter) Pitch(Pitch::DiatonicStep::step, (octave), Pitch::Alter::alter)
using namespace std;
using namespace m32;

TEST_CASE("pitch internal") {
    SECTION("test enharmonics") {
        REQUIRE(MAKE_PITCH(A, 4, Sharp).ps == MAKE_PITCH(B, 4, Flat).ps);
        REQUIRE(MAKE_PITCH(B, 3, Sharp).ps == MAKE_PITCH(C, 4, No).ps);
        REQUIRE(MAKE_PITCH(E, 4, No).ps == MAKE_PITCH(F, 4, Flat).ps);
        REQUIRE(MAKE_PITCH(E, 4, Sharp).ps == MAKE_PITCH(F, 4, No).ps);
        REQUIRE(MAKE_PITCH(B, 3, No).ps == MAKE_PITCH(C, 4, Flat).ps);
        REQUIRE(MAKE_PITCH(G, 4, Sharp).ps == MAKE_PITCH(A, 4, Flat).ps);
        REQUIRE(MAKE_PITCH(D, 4, Flat).ps == MAKE_PITCH(C, 4, Sharp).ps);
    }

    SECTION("test transposition") {
        SECTION("up") {
            vector<Pitch> before (15, MAKE_PITCH(A, 3, No));
            vector<Pitch> after = {
                    MAKE_PITCH(A, 3, No),
                    MAKE_PITCH(A, 3, Sharp),
                    MAKE_PITCH(B, 3, No),
                    MAKE_PITCH(C, 4, No),
                    MAKE_PITCH(C, 4, Sharp),
                    MAKE_PITCH(D, 4, No),
                    MAKE_PITCH(D, 4, Sharp),
                    MAKE_PITCH(E, 4, No),
                    MAKE_PITCH(F, 4, No),
                    MAKE_PITCH(F, 4, Sharp),
                    MAKE_PITCH(G, 4, No),
                    MAKE_PITCH(G, 4, Sharp),
                    MAKE_PITCH(A, 4, No),
                    MAKE_PITCH(A, 4, Sharp),
                    MAKE_PITCH(B, 4, No),
            };
            for (int i = 0; i < 15; i ++) {
                before[i].transpose(i);
                REQUIRE(before[i].ps == after[i].ps);
            }
        }
        SECTION("down") {
            vector<Pitch> before (15, MAKE_PITCH(A, 4, No));
            vector<Pitch> after = {
                    MAKE_PITCH(A, 4, No),
                    MAKE_PITCH(G, 4, Sharp),
                    MAKE_PITCH(G, 4, No),
                    MAKE_PITCH(F, 4, Sharp),
                    MAKE_PITCH(F, 4, No),
                    MAKE_PITCH(E, 4, No),
                    MAKE_PITCH(E, 4, Flat),
                    MAKE_PITCH(D, 4, No),
                    MAKE_PITCH(D, 4, Flat),
                    MAKE_PITCH(C, 4, No),
                    MAKE_PITCH(B, 3, No),
                    MAKE_PITCH(B, 3, Flat),
                    MAKE_PITCH(A, 3, No),
                    MAKE_PITCH(A, 3, Flat),
                    MAKE_PITCH(G, 3, No)
            };
            for (int i = 0; i < 15; i ++) {
                before[i].transpose(-i);
                REQUIRE(before[i].ps == after[i].ps);
            }
        }
    }
}