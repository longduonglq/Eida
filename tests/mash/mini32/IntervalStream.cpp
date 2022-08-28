// Project AIDA
// Created by Long Duong on 7/12/22.
// Purpose: 
//

#include <catch2/catch.hpp>
#include <simdpp/simd.h>
#include <iostream>
#include <mash/mini32/IntervalStream.h>
#include <mash/mini32/PcC.h>
#include <stdint.h>
#include <tinynote/SimpleNoteParser.h>
#include <tinynote/MeasureParser.h>
#include <tinynote/PartParser.h>

TEST_CASE("ma") {
    const char* sf = "fsdf";
    SIMDPP_ALIGN(16) int16_t dat[32] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 ,13, 14, 15, 16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 ,13, 14, 15, 16};
    simdpp::float32<32> vecs;
    for (int i = 0; i < 100; i++) {
        simdpp::int16<32> vec = simdpp::load(dat);
        simdpp::int16<32> vec2 = simdpp::load(dat);
        simdpp::float32<32> f = simdpp::to_float32(simdpp::add_sat(vec, vec2));
        vecs = simdpp::sqrt(f);
        vecs = simdpp::mul(f, vecs);
    }

    SIMDPP_ALIGN(16) float_t res[32];
    simdpp::store(res, vecs);
}

TEST_CASE("PcC") {
    SECTION("from pitch collection") {
        SECTION("test 1") {
            auto ch = tinynote::parse_simple_note("Ch[C4, E5, G#6]");
            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
            REQUIRE(pcc.as_underlying_t() == 273);
            // std::cout << pcc << pcc.as_underlying_t() << std::endl;
        }
        SECTION("test 2") {
            auto ch = tinynote::parse_simple_note("Ch[C4, C4, E5, G#6]");
            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
            REQUIRE(pcc.as_underlying_t() == 273);
            // std::cout << pcc << pcc.as_underlying_t() << std::endl;
        }
        SECTION("test 3") {
            auto ch = tinynote::parse_simple_note("Ch[Cb4, C4, E5, G#6]");
            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
            REQUIRE(pcc.as_underlying_t() == 2321);
            // std::cout << pcc << pcc.as_underlying_t() << std::endl;
            SECTION("clear-pc") {
                REQUIRE(pcc.does_contain(mini32::PcC::E));
                pcc.clear_pc(mini32::PcC::E);
                // std::cout << pcc << pcc.as_underlying_t() << std::endl;
                REQUIRE_FALSE(pcc.does_contain(mini32::PcC::E));
                REQUIRE(pcc.as_underlying_t() == 2305);
            }
        }
    }
    SECTION("absorb-pcc") {
        auto ch = tinynote::parse_simple_note("Ch[C4, E5, G#6]");
        auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
        REQUIRE(pcc.as_underlying_t() == 273);
        // std::cout << pcc << pcc.as_underlying_t() << std::endl;

        auto ch1 = tinynote::parse_simple_note("Ch[Cb4, C4, E5, G#6]");
        auto pcc1 = mini32::PcC::from_pitch_collection(ch1.pitches);
        pcc.absorb_pcc(pcc1);
        REQUIRE(pcc.as_underlying_t() == 2321);

        SECTION("clear-pitches & does_contain") {
            REQUIRE(pcc.does_contain(mini32::PcC::Pc::C));
            pcc.clear_pitches();
            // std::cout << pcc << pcc.as_underlying_t() << std::endl;
            REQUIRE_FALSE(pcc.does_contain(mini32::PcC::Pc::C));
            REQUIRE(pcc.is_empty());
        }
    }
    SECTION("combine-with-pcc") {
        auto ch = tinynote::parse_simple_note("G#6:2");
        auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
        REQUIRE(pcc.as_underlying_t() == 256);
        // std::cout << pcc << pcc.as_underlying_t() << std::endl;

        auto ch1 = tinynote::parse_simple_note("Cb4:3");
        auto pcc1 = mini32::PcC::from_pitch_collection(ch1.pitches);
        auto combined = pcc.combine_with_pcc(pcc1);
        // std::cout << pcc << pcc.as_underlying_t() << std::endl;
        REQUIRE(combined.as_underlying_t() == 2304);

        SECTION("add-pc") {
            pcc.add_pc(mini32::PcC::E);
            // std::cout << pcc << pcc.as_underlying_t() << std::endl;
            REQUIRE(pcc.as_underlying_t() == 272);
        }
    }
    SECTION("pcs-as-set") {
        auto ch = tinynote::parse_simple_note("Ch[C4, E5, G#6]");
        auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
        auto set = pcc.pcs_as_set();
        // for (auto b : set) std::cout << b << std::endl;
        REQUIRE((
                set.size() == 3 &&
                set[0] == mini32::PcC::C &&
                        set[1] == mini32::PcC::E &&
                        set[2] == mini32::PcC::Ab
                ));
    }
    SECTION("transpose-pc-by") {
        {
            auto pc = mini32::PcC::transpose_pc_by(mini32::PcC::E, 2);
            REQUIRE((pc == Ith_PCLS(6)));
        }
        {
            auto pc = mini32::PcC::transpose_pc_by(mini32::PcC::E, 12);
            REQUIRE((pc == Ith_PCLS(4)));
        }
        {
            auto pc = mini32::PcC::transpose_pc_by(mini32::PcC::E, 11);
            REQUIRE((pc == mini32::PcC::Eb));
        }
        {
            auto pc = mini32::PcC::transpose_pc_by(mini32::PcC::E, -2);
            REQUIRE((pc == mini32::PcC::D));
        }
        {
            auto pc = mini32::PcC::transpose_pc_by(mini32::PcC::B, 13);
            REQUIRE((pc == mini32::PcC::C));
        }
        {
            auto pc = mini32::PcC::transpose_pc_by(mini32::PcC::E, -14);
            REQUIRE((pc == mini32::PcC::D));
        }
    }
    SECTION("transpose-pc") {
        {
            auto ch = tinynote::parse_simple_note("Ch[C4, E5, G#6]");
            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
            pcc.transpose_pc(mini32::PcC::E, -1);
            // std::cout << pcc << pcc.as_underlying_t() ;
            REQUIRE(pcc.as_underlying_t() == 265);
            SECTION("pc do not exist") {
                REQUIRE_THROWS(pcc.transpose_pc(mini32::PcC::G, 2));
                // std::cout << pcc << pcc.as_underlying_t();
            }
        }
    }
    SECTION("transpose-whole-by") {
        {
            auto ch = tinynote::parse_simple_note("Ch[C4, E5, G#6]");
            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
            pcc.transpose_whole_by(2);
            // std::cout << pcc << pcc.as_underlying_t();
            REQUIRE(pcc.as_underlying_t() == 1092);
        }
        {
            auto ch = tinynote::parse_simple_note("Ch[C4, E5, G#6]");
            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
            pcc.transpose_whole_by(5);
            // std::cout << pcc << pcc.as_underlying_t();
            REQUIRE(pcc.as_underlying_t() == 546);

        }
    }

    SECTION("chord-type-map") {
        SECTION("test 1") {
            auto ctm = mini32::ChordTypeMap();
            auto c = mini32::PcC{ctm.convert({
                {mini32::ChordTypeMap::AccidentalType::No, 1},
                {mini32::ChordTypeMap::AccidentalType::Sharp, 4}
            })};
            // std::cout << c << c.as_underlying_t() << std::endl;
            REQUIRE(c.as_underlying_t() == 65);
        }
        SECTION("test 1") {
            auto ch = tinynote::parse_simple_note("Ch[C4, E5, G6]");
            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
            auto type = pcc.chord_type();
            REQUIRE(type == heval::Major);
        }
        SECTION("test 2") {
            auto ch = tinynote::parse_simple_note("Ch[C4, Eb5, G6]");
            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
            auto type = pcc.chord_type();
            REQUIRE(type == heval::Minor);
        }
        SECTION("test 1") {
            auto ch = tinynote::parse_simple_note("Ch[C4, Eb5, Gb6]");
            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
            auto type = pcc.chord_type();
            REQUIRE(type == heval::Diminished);
        }
//        SECTION("test 1") {
//            auto ch = tinynote::parse_simple_note("Ch[C4, E5, G6, Bb4]");
//            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
//            auto type = pcc.chord_type();
//            auto ctm = mini32::ChordTypeMap();
//            REQUIRE(type == heval::Dominant7th);
//        }
//        SECTION("test 1") {
//            auto ch = tinynote::parse_simple_note("Ch[C4, E5, G#6, B4]");
//            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
//            auto type = pcc.chord_type();
//            REQUIRE(type == heval::AugmentedMajor7th);
//        }
//        SECTION("test 1") {
//            auto ch = tinynote::parse_simple_note("Ch[C4, E5, G6, A6]");
//            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
//            auto type = pcc.chord_type();
//            REQUIRE(type == heval::Major6th);
//        }
//        SECTION("test 1") {
//            auto ch = tinynote::parse_simple_note("Ch[C4, E5, G6, B6, D6]");
//            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
//            auto type = pcc.chord_type();
//            REQUIRE(type == heval::Major9th);
//        }
//        SECTION("test 1") {
//            auto ch = tinynote::parse_simple_note("Ch[C4, Eb5, Gb6, A6, D6]");
//            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
//            auto type = pcc.chord_type();
//            REQUIRE(type == heval::Diminished9th);
//        }
//        SECTION("test 1") {
//            auto ch = tinynote::parse_simple_note("Ch[C4, Eb5, G6, Bb6, D6, F6, A6]");
//            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
//            auto type = pcc.chord_type();
//            REQUIRE(type == heval::Minor13th);
//        }
//        SECTION("test 1") {
//            auto ch = tinynote::parse_simple_note("Ch[C4, D4, F#4, Ab6]");
//            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
//            auto type = pcc.chord_type();
//            REQUIRE(type == heval::French);
//        }
//        /// different keys
//        SECTION("test 1") {
//            auto ch = tinynote::parse_simple_note("Ch[C#4, F6, G#6]");
//            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
//            auto type = pcc.chord_type();
//            REQUIRE(type == heval::Major);
//        }
//        SECTION("test 2") {
//            auto ch = tinynote::parse_simple_note("Ch[C#4, E5, G#6]");
//            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
//            auto type = pcc.chord_type();
//            REQUIRE(type == heval::Minor);
//        }
//        SECTION("test 1") {
//            auto ch = tinynote::parse_simple_note("Ch[C#4, E5, G6]");
//            auto pcc = mini32::PcC::from_pitch_collection(ch.pitches);
//            auto type = pcc.chord_type();
//            REQUIRE(type == heval::Diminished);
//        }
    }
}

template <typename T>
using AlignedVec = typename mini32::BindAlloc<simdpp::aligned_allocator<T, M_ALIGN> , std::vector>::template type<T>;

struct break_debug : public mini32::BreakIfOverlapAllZero {
    int64_t res_when_break = 0;

    template <typename ResT, typename OverlapT>
    bool operator()(ResT res, OverlapT overlap) {
        bool br = mini32::BreakIfOverlapAllZero::operator()(res, overlap);
        if (br) res_when_break = res;
        return br;
    }
};

TEST_CASE("interval-stream") {
    using interval_stream_t = mini32::IntervalStream<AlignedVec>;
    using gnote_t = m32::GNote<m32::Offset, m32::Duration>;
    using part_t = m32::Part<gnote_t>;
    SECTION("from-part") {
        auto part = tinynote::parse_part<m32::Offset,m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    rest:2[tie_info=TieStart+TieEnd],
    C4:3[tie_info=TieEnd+TieStart],
    C4:1.5[tie_info=TieEnd],

    Tup@2!3[
        C4:1[tie_info=TieStart],
        C4:1[tie_info=TieEnd+TieStart],
        C4:1[tie_info=TieEnd]
    ]:1
}
)");
        auto stream = interval_stream_t::from_part(part);
        using stream_interval = interval_stream_t::IntervalT;

        int starts[5] = {12, 30, 39, 41, 43};
        int ends[5] = {30, 39, 41, 43, 45};
        REQUIRE(std::equal(stream.itv_starts.begin(), stream.itv_starts.end(), starts));
        REQUIRE(std::equal(stream.itv_ends.begin(), stream.itv_ends.end(), ends));

        // std::cout << stream << std::endl;
        int16_t arr[32];
        std::fill(arr, arr + 32, 1);

        SECTION("fold") {
            SECTION("test 1") {
               auto res = stream.fold<
                    int64_t,
                    mini32::DotProduct<int64_t, 16>,
//                    mini32::NeverBreakEarly,
                    mini32::DebugNone
                    >(
                            stream_interval::from_end_points(35, 41),
                            0,
                            mini32::DotProduct<int64_t, 16>{arr});
                // std::cout << stream << std::endl;
                // std::cout << res << std::endl;
                REQUIRE(res == 6);
            }SECTION("test 2") {
                auto res = stream.fold<
                        int64_t,
                        mini32::DotProduct<int64_t, 16>,
//                        mini32::NeverBreakEarly,
                        mini32::DebugNone
                >(
                        stream_interval::from_end_points(0, 41),
                        0,
                        mini32::DotProduct<int64_t, 16>{arr});
                // std::cout << stream << std::endl;
                // std::cout << res << std::endl;
                REQUIRE(res == 29);
            }SECTION("test 3") {
                auto res = stream.fold<
                        int64_t,
                        mini32::DotProduct<int64_t, 16>,
//                        mini32::NeverBreakEarly,
                        mini32::DebugNone
                >(
                        stream_interval::from_end_points(38, 42),
                        0,
                        mini32::DotProduct<int64_t, 16>{arr});
                //std::cout << stream << std::endl;
                // std::cout << res << std::endl;
                REQUIRE(res == 4);
            }SECTION("test 4") {
                auto res = stream.fold<
                        int64_t,
                        mini32::DotProduct<int64_t, 16>,
//                        mini32::NeverBreakEarly,
                        mini32::DebugNone
                >(
                        stream_interval::from_end_points(35, 45),
                        0,
                        mini32::DotProduct<int64_t, 16>{arr});
                // std::cout << stream << std::endl;
                // std::cout << res << std::endl;
                REQUIRE(res == 10);
            }SECTION("test 5") {
                auto res = stream.fold<
                        int64_t,
                        mini32::DotProduct<int64_t, 16>,
//                        mini32::NeverBreakEarly,
                        mini32::DebugNone
                >(
                        stream_interval::from_end_points(0, 46),
                        0,
                        mini32::DotProduct<int64_t, 16>{arr});
                // std::cout << stream << std::endl;
                // std::cout << res << std::endl;
                REQUIRE(res == 33);
            }
        }
    }

    SECTION("to-part") {
        SECTION("test 1") {
            //TODO: incomplete test
            auto part = tinynote::parse_part<m32::Offset, m32::Duration>(R"(
Part[clef=treble, key=0, time=4/4] {
    rest:2[tie_info=TieStart+TieEnd],
    C4:3[tie_info=TieEnd+TieStart],
    C4:1.5[tie_info=TieEnd]
}
)");
            auto stream = interval_stream_t::from_part(part);
            using stream_interval = interval_stream_t::IntervalT;
            auto retrieved_part = stream.to_part<part_t>();
            // std::cout << stream << std::endl;
            // std::cout << part << std::endl;
            // std::cout << retrieved_part << std::endl;
            // REQUIRE(part.exact_eq(retrieved_part));
        }
    }
}