// Project AIDA
// Created by Long Duong on 5/23/22.
// Purpose: 
//

#include <catch2/catch.hpp>
#include <algorithm>
#include <botan/filters.h>
#include <boost/iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/tuple/tuple.hpp>

#include "m32/Part.h"
#include "m32/Measure.h"
#include "tinynote/PartParser.h"
#include "tinynote/SimpleNoteParser.h"
#include "tinynote/TupletParser.h"
#include "show.h"

using gnote_t = m32::GNote<m32::Offset, m32::Duration>;
using part_t = m32::Part<gnote_t>;

using namespace tinynote;

TEST_CASE("nonMeasured-part-test") {
    SECTION("to-measured") {
        auto part = parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    C4:2,
    E4:3,
    G5:9
}
)");

        auto mpart = part.to_measured();

        auto m1 = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[length=4, start=0]{
    C4:2, E4:2[tie_info=TieStart]
}
)");

        auto m2 = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[length=4, start=4]{
    E4:1[tie_info=TieEnd], G5:3[tie_info=TieStart]
}
)");
        auto m3 = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[length=4, start=8]{
    G5:4[tie_info=TieStart+TieEnd]
}
)");
        auto m4 = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[length=4, start=12]{
    G5:2[tie_info=TieEnd]
}
)");
        m1.measure_number = 0;
        m2.measure_number = 1;
        m3.measure_number = 2;
        m4.measure_number = 3;

        REQUIRE(mpart.measures[0].exact_eq(m1));
        REQUIRE(mpart.measures[1].exact_eq(m2));
        REQUIRE(mpart.measures[2].exact_eq(m3));
        REQUIRE(mpart.measures[3].exact_eq(m4));
    }



    SECTION("to-measured-with-tuplet") {
        auto part = parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    C4:3.25,
    Tup@2!3[E4:1, E4:1, E4:1]:1
}
)");

        auto mpart = part.to_measured();

        auto m1 = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[length=4, start=0]{
    C4:3.25,
    Tup@6!9[E4:1/3, E4:1/3, E4:1/12[tie_info=TieStart]]:0.75
}
)");

        auto m2 = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[length=4, start=4]{
    E4:0.25[tie_info=TieEnd]
}
)");

        m1.measure_number = 0;
        m2.measure_number = 1;

        REQUIRE(mpart.measures[0].exact_eq(m1));
        REQUIRE(mpart.measures[1].exact_eq(m2));
    }

    SECTION("to-measured-perfect-fit") {
        auto part = parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    C4:3.25,
    Tup@2!3[E4:1, E4:1, E4:1]:0.75
}
)");

        auto mpart = part.to_measured();

        auto m1 = parse_measure<m32::Offset, m32::Duration>(R"(
Mea[length=4, start=0]{
    C4:3.25,
    Tup@2!3[E4:1/3, E4:1/3, E4:1/3]:0.75
}
)");

        m1.measure_number = 0;
        REQUIRE(mpart.measures[0].exact_eq(m1));
    }


    SECTION("simple-note-iter") {
        SECTION("1") {
            auto part = parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    C4:3.25,
    Tup@2!3[E4:1, E4:1, E4:1]:1
}
)");
            auto flat = std::vector<m32::SimpleNote<m32::Offset, m32::Duration>> {
                    parse_simple_note("C4:3.25[start=0]"),
                    parse_simple_note("E4:1/3[start=3.25]"),
                    parse_simple_note("E4:1/3[start=43/12]"),
                    parse_simple_note("E4:1/3[start=47/12]")
            };

            auto m = std::vector<m32::SimpleNote<m32::Offset, m32::Duration>>();
            for (auto& sn : part.simple_note_iter()) {
                m.push_back(sn);
            }
            REQUIRE(std::equal(m.begin(), m.end(), flat.begin(),
                       [](auto& a, auto& b) { return a.exact_eq(b); }));
        }
    }

    SECTION("simple-note-itermut") {
        SECTION("1") {
            auto part = parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    C4:3.25,
    Tup@2!3[E4:1, E4:1, E4:1]:1
}
)");
            auto flat = std::vector<m32::SimpleNote<m32::Offset, m32::Duration>> {
                    parse_simple_note("C4:3.25[start=1]"),
                    parse_simple_note("E4:1/3[start=4.25]"),
                    parse_simple_note("E4:1/3[start=55/12]"),
                    parse_simple_note("E4:1/3[start=59/12]")
            };

            auto m = std::vector<m32::SimpleNote<m32::Offset, m32::Duration>>();
            for (auto& sn : part.simple_note_itermut()) {
                sn.displace_start<KeepLength>(1);
                m.push_back(sn);
            }
            REQUIRE(std::equal(m.begin(), m.end(), flat.begin(),
                               [](auto& a, auto& b) { return a.exact_eq(b); }));

        }
    }

    SECTION("transpose-by") {
        SECTION("test 1") {
            auto part = parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    C4:3.25,
    Tup@2!3[E4:1, E4:1, E4:1]:1
})");
            auto sp = part.transpose_by(1);
            auto spart = parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    C#4:3.25,
    Tup@2!3[F4:1, F4:1, F4:1]:1
})");
            // std::cout << part << sp << spart << std::endl;
            REQUIRE(sp._eq<
                    part_t::HashCmp<>,
                    gnote_t::StartCmp,
                    gnote_t::EndCmp,
                    gnote_t::LengthCmp,
                    gnote_t::SimpleNoteT::PitchCmpPs
                    >(spart)
                    );
        }
    }
    SECTION("hashing") {
        SECTION("sorting hashers") {
            auto part = parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieEnd] }
)");
            auto v = part.hash_self<part_t::HashKeySig, part_t::HashTimeSig, part_t::HashDuration, part_t::HashPitch>();

            auto v1 = part.hash_self<part_t::HashDuration, part_t::HashPitch, part_t::HashTimeSig, part_t::HashKeySig>();

            REQUIRE(std::equal(v.begin(), v.end(), v1.begin()));
        }
    }

    SECTION("fuse-tie") {
        SECTION("test 1") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieEnd] }
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:3.5 }
)");
            auto fused = part.fuse_tied_notes();
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 2") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieStart+TieEnd], C4:0.5[tie_info=TieEnd] }
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:4 }
)");
            auto fused = part.fuse_tied_notes();
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 3") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieStart+TieEnd], C4:0.5[tie_info=TieEnd], rest:3.5 }
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:4, rest:3.5}
)");
            auto fused = part.fuse_tied_notes();
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 4 - with tuplet") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieStart+TieEnd], C4:0.5[tie_info=TieEnd], rest:3.5, Tup@2!3[C4:0.5[tie_info=TieStart], C4:0.25[tie_info=TieEnd], rest:0.5]:2 }
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:4, rest:3.5, Tup@2!3[C4:0.75, rest:0.5]:2}
)");
            auto fused = part.fuse_tied_notes();
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 5 - with tuplet") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
rest:4,
C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieStart+TieEnd], C4:0.5[tie_info=TieEnd],
rest:3.5,
Tup@2!3[
    C4:0.5[tie_info=TieStart],
    C4:0.25[tie_info=TieEnd+TieStart],
    C4:0.5[tie_info=TieEnd],
    rest:0.5,
    C4:0.5[tie_info=TieStart],
    C4:0.25[tie_info=TieEnd+TieStart],
    C4:0.5[tie_info=TieEnd],
    rest:0.1
]:2
}
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:4, rest:3.5, Tup@2!3[C4:1.25, rest:0.5, C4:1.25, rest:0.1]:2}
)");
            auto fused = part.fuse_tied_notes();
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 6 - with tuplet - do not join tuplet component with outside node") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
rest:4,
C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieStart+TieEnd], C4:0.5[tie_info=TieEnd],
rest:3.5,
Tup@2!3[
    C4:0.5[tie_info=TieStart],
    C4:0.25[tie_info=TieEnd+TieStart],
    C4:0.5[tie_info=TieEnd],
    rest:0.5,
    C4:0.5[tie_info=TieStart],
    C4:0.25[tie_info=TieEnd+TieStart],
    C4:0.5[tie_info=TieEnd],
    rest:0.1[tie_info=TieStart]
]:2,
rest:0.4[tie_info=TieEnd]}
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:4, rest:3.5, Tup@2!3[C4:1.25, rest:0.5, C4:1.25, rest:0.1[tie_info=TieStart]]:2, rest:0.4[tie_info=TieEnd] }
)");
            auto fused = part.fuse_tied_notes();
            // std::cout << part << std::endl << fpart << std::endl << fused << std::endl;
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 7 - with tuplet - do not join tuplet component with outside node") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    rest:4,
    C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieStart+TieEnd], C4:0.5[tie_info=TieEnd],
    C4:4[tie_info=TieStart],
    Tup@2!3[
        C4:0.5[tie_info=TieStart+TieEnd],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],
        rest:0.5,
        C4:0.5[tie_info=TieStart],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],
        rest:0.1[tie_info=TieStart]
    ]:2,
    rest:0.4[tie_info=TieEnd]}
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    rest:4,
    C4:4,
    C4:4[tie_info=TieStart],
    Tup@2!3[
        C4:1.25[tie_info=TieEnd],
        rest:0.5,
        C4:1.25,
        rest:0.1[tie_info=TieStart]
    ]:2,
    rest:0.4[tie_info=TieEnd]
}
)");
            auto fused = part.fuse_tied_notes();
            // std::cout << part << std::endl << fpart << std::endl << fused << std::endl;
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 8 - with tuplet - do not fuse without appropriate tie_info") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    rest:4,
    C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieStart+TieEnd], C4:0.5[tie_info=TieEnd],
    C4:4[tie_info=TieStart],
    Tup@2!3[
        C4:0.5[tie_info=TieStart+TieEnd],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],

        C4:0.5[tie_info=TieStart],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],
        rest:0.1[tie_info=TieStart]
    ]:2,
    rest:0.4[tie_info=TieEnd]}
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    rest:4,
    C4:4,
    C4:4[tie_info=TieStart],
    Tup@2!3[
        C4:1.25[tie_info=TieEnd],

        C4:1.25,
        rest:0.1[tie_info=TieStart]
    ]:2,
    rest:0.4[tie_info=TieEnd]
}
)");
            auto fused = part.fuse_tied_notes();
            // std::cout << part << std::endl << fpart << std::endl << fused << std::endl;
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 9 - with tuplet - don't fuse two different tuplets") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    rest:4,
    C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieStart+TieEnd], C4:0.5[tie_info=TieEnd],
    C4:4[tie_info=TieStart],
    Tup@2!3[
        C4:0.5[tie_info=TieStart+TieEnd],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],

        rest:0.1,

        C4:0.5[tie_info=TieStart],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd+TieStart]
    ]:2,
    Tup@2!3[
        C4:0.5[tie_info=TieStart+TieEnd],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],

        C4:0.5[tie_info=TieStart],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],

        rest:0.1[tie_info=TieStart]
    ]:3,
    rest:0.4[tie_info=TieEnd]
}
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    rest:4,
    C4:4,
    C4:4[tie_info=TieStart],
    Tup@2!3[
        C4:1.25[tie_info=TieEnd],
        rest:0.1,
        C4:1.25[tie_info=TieStart]
    ]:2,
    Tup@2!3[
        C4:1.25[tie_info=TieEnd],
        C4:1.25,
        rest:0.1[tie_info=TieStart]
    ]:3,
    rest:0.4[tie_info=TieEnd]
}
)");
            auto fused = part.fuse_tied_notes();
            // std::cout << part << std::endl << fpart << std::endl << fused << std::endl;
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 10 - tie but not fusable") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:3.25[tie_info=TieStart], C5:0.25[tie_info=TieEnd] }
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:3.25[tie_info=TieStart], C5:0.25[tie_info=TieEnd] }
)");
            auto fused = part.fuse_tied_notes();
            // std::cout << part << fpart << fused << std::endl;
            REQUIRE(fused.exact_eq(fpart));
        }


        SECTION("test 11 - with tuplet - tie but not fusable") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    rest:4,
    C4:3.25[tie_info=TieStart], C4:0.25[tie_info=TieStart+TieEnd], C4:0.5[tie_info=TieEnd],
    C4:4[tie_info=TieStart],
    Tup@2!3[
        rest:0.5[tie_info=TieStart+TieEnd],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],

        rest:0.1,

        C4:0.5[tie_info=TieStart],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd+TieStart]
    ]:2,
    Tup@2!3[
        C4:0.5[tie_info=TieStart+TieEnd],
        C4:0.25[tie_info=TieEnd+TieStart],
        C5:0.5[tie_info=TieEnd],

        C4:0.5[tie_info=TieStart],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],

        rest:0.1[tie_info=TieStart]
    ]:3,
    rest:0.4[tie_info=TieEnd]
}
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    rest:4,
    C4:4,
    C4:4[tie_info=TieStart],
    Tup@2!3[
        rest:0.5[tie_info=TieStart+TieEnd],
        C4:0.75[tie_info=TieEnd],
        rest:0.1,
        C4:1.25[tie_info=TieStart]
    ]:2,
    Tup@2!3[
        C4:0.75[tie_info=TieEnd+TieStart],
        C5:0.5[tie_info=TieEnd],
        C4:1.25,
        rest:0.1[tie_info=TieStart]
    ]:3,
    rest:0.4[tie_info=TieEnd]
}
)");
            auto fused = part.fuse_tied_notes();
            // std::cout << part << std::endl << fpart << std::endl << fused << std::endl;
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 13 - TieStart at the end") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:3.25[tie_info=TieStart], C5:0.25[tie_info=TieStart] }
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { rest:4, C4:3.25[tie_info=TieStart], C5:0.25[tie_info=TieStart] }
)");
            auto fused = part.fuse_tied_notes();
            // std::cout << part << fpart << fused << std::endl;
            REQUIRE(fused.exact_eq(fpart));
        }

        SECTION("test 14 - Begins and ends with tuplets") {
            auto part = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    Tup@2!3[
        rest:0.5[tie_info=TieStart+TieEnd],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],

        rest:0.1,

        C4:0.5[tie_info=TieStart],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd+TieStart]
    ]:2,
    Tup@2!3[
        C4:0.5[tie_info=TieStart+TieEnd],
        C4:0.25[tie_info=TieEnd+TieStart],
        C5:0.5[tie_info=TieEnd],

        C4:0.5[tie_info=TieStart],
        C4:0.25[tie_info=TieEnd+TieStart],
        C4:0.5[tie_info=TieEnd],

        rest:0.1[tie_info=TieStart]
    ]:3
}
)");
            auto fpart = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    Tup@2!3[
        rest:0.5[tie_info=TieStart+TieEnd],
        C4:0.75[tie_info=TieEnd],
        rest:0.1,
        C4:1.25[tie_info=TieStart]
    ]:2,
    Tup@2!3[
        C4:0.75[tie_info=TieEnd+TieStart],
        C5:0.5[tie_info=TieEnd],
        C4:1.25,
        rest:0.1[tie_info=TieStart]
    ]:3
}
)");
            auto fused = part.fuse_tied_notes();
            // std::cout << part << std::endl << fpart << std::endl << fused << std::endl;
            REQUIRE(fused.exact_eq(fpart));
        }
    }

    SECTION("_eq") {
        auto p1 = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    Tup@2!3[
        rest:0.5[tie_info=TieStart+TieEnd],
        C4:0.75[tie_info=TieEnd],
        rest:0.1,
        C4:1.25[tie_info=TieStart]
    ]:2,
    Tup@2!3[
        C4:0.75[tie_info=TieEnd+TieStart],
        C5:0.5[tie_info=TieEnd],
        C4:1.25,
        rest:0.1[tie_info=TieStart]
    ]:3
}
)");
        auto p2 = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    Tup@2!3[
        rest:0.5[tie_info=TieStart+TieEnd],
        C4:0.75[tie_info=TieEnd],
        rest:0.1,
        C4:1.25[tie_info=TieStart]
    ]:2,
    Tup@2!3[
        C4:0.75[tie_info=TieEnd],
        B#4:0.5[tie_info=TieEnd],
        C4:1.25,
        rest:0.1[tie_info=TieStart]
    ]:3
}
)");
        REQUIRE(p1._eq<
            part_t::HashCmp<>,
            part_t::PartNameCmp,
            part_t::KeySigCmp,
            part_t::TimeSigCmp,

            gnote_t::SimpleNoteT::PitchCmpPs,

            gnote_t::TupletT::NormalActualCmp
        >(p2));

        REQUIRE_FALSE(p1._eq<
                part_t::HashCmp<>,
                part_t::PartNameCmp,
                part_t::KeySigCmp,
                part_t::TimeSigCmp,

                gnote_t::SimpleNoteT::PitchCmpExact,
                gnote_t::SimpleNoteT::TieInfoCmp,

                gnote_t::TupletT::NormalActualCmp
        >(p2));
    }

    SECTION("split-at-offsets") {
        SECTION("test 1") {
            auto p2 = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { C4:2, D4:1, E4:4} )");
            // std::cout << p2 << std::endl;
            std::vector<m32::Offset> offsets = {{1, 1}, {25, 10}};
            p2.split_at_offsets<part_t::CreateTies>(offsets);
            // std::cout << p2 << std::endl;
            auto p3 = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
C4:1[tie_info=TieStart],
C4:1[tie_info=TieEnd],
D4:0.5[tie_info=TieStart],
D4:0.5[tie_info=TieEnd],
E4:4
} )");
            // std::cout << p3 << std::endl;
            REQUIRE(p2.exact_eq(p3));
        }

        SECTION("test 2") {
            auto p2 = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] { C4:2, Tup@2!3[D4:1, D4:1, D4:1]:1, E4:4} )");
            /// std::cout << p2 << std::endl;
            std::vector<m32::Offset> offsets = {{1, 1}, {25, 10}};
            p2.split_at_offsets<part_t::CreateTies>(offsets);
            // std::cout << p2 << std::endl;
            auto p3 = parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
C4:1[tie_info=TieStart],
C4:1[tie_info=TieEnd],

Tup@2!3[D4:2, D4:1[tie_info=TieStart]]:0.5,
Tup@2!3[D4:1[tie_info=TieEnd], D4:2]:0.5,

E4:4
} )");
            // std::cout << p3 << std::endl;
            REQUIRE(p2.exact_eq(p3));
        }
    }
}

TEST_CASE("measured-part-test") {
    SECTION("flatten") {
        auto p1 = parse_measured_part(R"(
MeasuredPart[clef=treble, key=0, time=4/4] {
    Mea [time=4/4, clef=treble, key=4] {
        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
    },
    Mea {
        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
    }
}
)");
        auto flat = parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=4, time=4/4] {
        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2,

        Db4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
}
)");
        // std::cout << p1 << p1.flatten() << flat << std::endl;
        REQUIRE(p1.flatten()._eq<
            part_t::HashCmp<>,
            part_t::TimeSigCmp,
            part_t::ClefSignCmp,
            part_t::KeySigCmp,

            gnote_t::StartCmp,
            gnote_t::EndCmp,
            gnote_t::LengthCmp,
            gnote_t::SimpleNoteT::PitchCmpPs,

            gnote_t::TupletT::NormalActualCmp
        >(flat));

        REQUIRE_FALSE(p1.flatten()._eq<
                part_t::HashCmp<>,
                part_t::TimeSigCmp,
                part_t::ClefSignCmp,
                part_t::KeySigCmp,

                gnote_t::LengthCmp,
                gnote_t::SimpleNoteT::PitchCmpExact,
                gnote_t::SimpleNoteT::TieInfoCmp,

                gnote_t::TupletT::NormalActualCmp
        >(flat));
    }

    SECTION("simplenote-iterator") {
        auto p1 = parse_measured_part(R"(
MeasuredPart[clef=treble, key=0, time=4/4] {
    Mea [time=4/4, clef=treble, key=4] {
        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
    },
    Mea {
        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
    }
}
)");
        auto flat = parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=4, time=4/4] {
        C#4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2,

        Db4:2, Gb5:2,
        Ch[C4, G5, B6]:4[tie_info=TieStart],
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
}
)");
        auto p1vec = std::vector<typename gnote_t::SimpleNoteT >();
        for (const auto& a : p1.simple_note_iter()) { p1vec.push_back(a); }
        auto it1 = p1vec.begin();

        auto p2vec = std::vector<typename gnote_t::SimpleNoteT >();
        for (const auto& b : flat.simple_note_iter()) {p2vec.push_back(b); }
        auto it2 = p2vec.begin();

        auto zipbeg = boost::make_zip_iterator(boost::make_tuple(it1, it2));
        auto zipend = boost::make_zip_iterator(boost::make_tuple(p1vec.end(), p2vec.end()));

        auto all1 = std::all_of(
                zipbeg, zipend, [](const auto& it) {
                    // std::cout << it.template get<0>() << it.template get<1>() << std::endl;
                    using snote_t = gnote_t::SimpleNoteT;

                    auto n1 = it.template get<0>();
                    auto n2 = it.template get<1>();
                    return n1.template _eq<
                            snote_t::PitchCmpPs,
                            snote_t::LengthCmp,
                            snote_t::StartCmp
                    >(n2);
                });
        REQUIRE_FALSE(all1);

        auto all3 = std::all_of(
                zipbeg, zipend, [](const auto& it) {
                    // std::cout << it.template get<0>() << it.template get<1>() << std::endl;
                    using snote_t = gnote_t::SimpleNoteT;

                    auto n1 = it.template get<0>();
                    auto n2 = it.template get<1>();
                    return n1.template _eq<
                            snote_t::PitchCmpPs,
                            snote_t::LengthCmp
                    >(n2);
                });
        REQUIRE(all3);
    }

    SECTION("append-gnotes") {
        SECTION("simple-insert") {
            auto p1 = parse_measured_part(R"(
MeasuredPart[clef=treble, key=0, time=4/4] {
    Mea [time=4/4, clef=treble, key=4, length=4] {
        C#4:2, Gb5:2
    },
    Mea [length=4]{
        Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd]
    }
}
)");
            auto ip = parse_measured_part(R"(
MeasuredPart[clef=treble, key=0, time=4/4] {
    Mea [time=4/4, clef=treble, key=4, length=4] {
        C#4:2, Gb5:2
    }
}
)");
            // std::cout << p1 << ip << std::endl;
            ip.append_gnote(gnote_t(parse_simple_note("Ch[C4, G5, B6]:4[tie_info=TieStart+TieEnd]")));
            using mpart = decltype(p1);
            REQUIRE(ip._eq<
                    mpart::HashCmp<>,
                    gnote_t::SimpleNoteT::PitchCmpPs,
                    gnote_t::LengthCmp,
                    gnote_t::StartCmp,
                    gnote_t::EndCmp
            >(p1));
        }

        SECTION("insert with split") {
            auto ip = parse_measured_part(R"(
MeasuredPart[clef=treble, key=0, time=4/4] {
    Mea [time=4/4, clef=treble, key=4, length=4] {
        C#4:2, Gb5:1.5
    }
}
)");
            auto p1 = parse_measured_part(R"(
MeasuredPart[clef=treble, key=0, time=4/4] {
    Mea [time=4/4, clef=treble, key=4, length=4] {
        C#4:2, Gb5:1.5, C5:0.5
    },
    Mea [time=4/4, clef=treble, key=4, length=4] {
        C5:3.5
    }
}
)");
            ip.append_gnote(gnote_t(parse_simple_note("C5:4")));
            // std::cout << p1 << ip << std::endl;
            using mpart = decltype(p1);
            REQUIRE(ip._eq<
                    mpart::HashCmp<>,
                    gnote_t::SimpleNoteT::PitchCmpPs,
                    gnote_t::LengthCmp,
                    gnote_t::StartCmp,
                    gnote_t::EndCmp
            >(p1));
        }
    }
}