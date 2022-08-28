//
// Created by lucad on 12/18/21.
//

#ifndef AIDA_SIMPLENOTE_H
#define AIDA_SIMPLENOTE_H

#include <optional>
#include <type_traits>
#include <vector>

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/is_sorted.hpp>

#include <boost/fusion/container.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/iterator.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/fusion/functional.hpp>

#include <boost/fusion/algorithm/query.hpp>
#include <boost/fusion/include/query.hpp>

#include <boost/lambda/lambda.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/fold.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/optional.hpp>

#include "common/Interval.h"
#include "common/gconfig.h"
#include "TieInfo.h"
#include "Pitch.h"
#include "Lyric.h"
#include "Color.h"
#include "types.h"
#include "utility.h"
#include "cmps.h"

namespace m32 {
    using namespace boost;

    template<
            typename OffsetType,
            typename DurationType,
            template <typename, typename...> typename PitchCont = std::vector,
            template <typename...> typename LyricCont = std::vector>
    requires IsDifferenceTypeOf<DurationType, OffsetType>
    class SimpleNote : public Interval<OffsetType, DurationType> {
    public:
        using Self = SimpleNote<OffsetType, DurationType, PitchCont, LyricCont>;
        PitchCont<Pitch> pitches;
        LyricCont<Lyric> lyrics;
        TieInfo tie_info;

        std::optional<Color> color;
        std::optional<float> dynamic;

        SimpleNote() =default;
        SimpleNote(
                OffsetType offset,
                DurationType duration,
                LyricCont<Lyric> lyrics,
                std::optional<Color> color,
                TieInfo tie_info
        )
                : Interval<OffsetType>(offset, duration)
                , lyrics{std::move(lyrics)}
                , color{color}
                , tie_info{tie_info}
                {};

        [[nodiscard]]
        std::pair<SimpleNote, SimpleNote>
        split_at_offset(OffsetType split_offset) const {
            assert(Interval<OffsetType>::does_half_closed_interval_contain_offset(split_offset)
                   && "Offset must be in note to split");
            assert(this->start != split_offset && split_offset != this->end &&
                   "One of the result after splitting will be empty");

            SimpleNote left = *this, right = *this;
            left.template set_end<KeepStart>(split_offset);
            right.template set_start<KeepEnd>(split_offset);

            // set tie
            left.tie_info |= TieStart;
            right.tie_info |= TieEnd;

            return std::make_pair(left, right);
        }

        std::pair<SimpleNote, SimpleNote>
        split_and_sever_ties_at_offset(OffsetType split_offset) const {
            auto [left, right] = this->split_at_offset(split_offset);
            left.tie_info &= ~TieStart;
            right.tie_info &= ~TieEnd;
            return std::make_pair(left, right);
        }

        void append_lyric(std::string lyric_text) {
            assert(
                    boost::range::is_sorted(
                            lyrics,
                            [](const auto &l, const auto& l2) { return l.number < l2.number; })
            );
            lyrics.template emplace_back(
                    lyrics.empty() ? 0 : lyrics.back().number + 1,
                    std::move(lyric_text)
            );
        }

        void remove_lyric(typename LyricCont<Lyric>::iterator lyric_iter) {
            lyrics.erase(lyric_iter);
            int i = 0;
            for (auto &lyric: lyrics) lyric.number = i++;
            assert(boost::range::is_sorted(lyrics,
                                           [](const auto &l, const auto& l2)
                                           { return l.number < l2.number; }));
        }

        bool is_rest() const { return pitches.empty(); }
        bool is_note() const { return pitches.size() == 1; }
        bool is_chord() const { return pitches.size() > 1; }
        bool is_tuplet() const { return false; }
        bool is_simple_note() const { return true; }

        friend std::ostream &operator<<(std::ostream &os, const SimpleNote &self) {
            if (self.is_rest())
                os << '\t'
                   << fmt::format("Rest<[{:.2f}, {:.2f}) | length={:.2f}>",
                                  rational_to<float>(self.start),
                                  rational_to<float>(self.end),
                                  rational_to<float>(self.length));
            else if (self.is_note()) {
                std::string tie_repr;
                switch (self.tie_info) {
                    case TieStart:
                        tie_repr = "start";
                        break;
                    case TieEnd:
                        tie_repr = "end";
                        break;
                    case TieStart | TieEnd:
                        tie_repr = "start+end";
                        break;
                }
                os << '\t'
                   << fmt::format("Note<[{:.2f}, {:.2f}) | length={:.2f} | color={} |",
                                  rational_to<float>(self.start),
                                  rational_to<float>(self.end),
                                  rational_to<float>(self.length),
                                  self.color.has_value() ? self.color.value().to_hex_rgb() : "")
                   << *self.pitches.cbegin() << " | ps=" << self.pitches.cbegin()->ps
                   << (self.tie_info != TieNeither
                       ? fmt::format(" | Tie=\"{}\"", tie_repr)
                       : "")
                   << '>';
            } else {
                std::string tie_repr;
                switch (self.tie_info) {
                    case TieStart:
                        tie_repr = "start";
                        break;
                    case TieEnd:
                        tie_repr = "end";
                        break;
                    case TieStart | TieEnd:
                        tie_repr = "start+end";
                        break;
                }

                os << '\t'
                   << fmt::format("Chord<[{:.2f}, {:.2f} | length={:.2f} | {{",
                                  rational_to<float>(self.start),
                                  rational_to<float>(self.end),
                                  rational_to<float>(self.length));

                for (auto pitch = self.pitches.cbegin(); pitch != self.pitches.cend(); pitch++) {
                    os << *pitch << (std::next(pitch) != self.pitches.cend() ? ", " : "");
                }
                os
                    << "}"
                    << (self.tie_info != TieNeither
                        ? fmt::format(" | Tie=\"{}\"", tie_repr)
                        : "")
                    << ">";
            }
            return os;
        }

        template <typename T, T f>
        using F = typename CmpField<Self>::template F<T, f>;

        struct PitchCmpExact {
            constexpr static auto cmp = [](const Self& n1, const Self& n2) {
#ifdef SHOW_CMP_MISMATCH
                if (!std::equal(n1.pitches.cbegin(), n1.pitches.cend(), n2.pitches.cbegin(),
                                [](const auto &a, const auto &b) { return a.exact_eq(b); })) {
                    for (auto p1: n1.pitches) std::cout << p1 << std::endl;
                    for (auto p2: n2.pitches) std::cout << p2 << std::endl;
                }
#endif
                return std::equal(n1.pitches.cbegin(), n1.pitches.cend(), n2.pitches.cbegin(),
                                  [](const auto &a, const auto &b) { return a.exact_eq(b); });
            };
        };
        struct PitchCmpPs {
            constexpr static auto cmp = [](const Self& n1, const Self& n2) {
                return std::equal(n1.pitches.cbegin(), n1.pitches.cend(), n2.pitches.cbegin(), [](auto &a, auto &b) { return a.ps_eq(b); });
            };
        };
        struct LyricCmp{ constexpr static auto cmp = [](const auto& n1, const auto n2){ return std::equal(n1.lyrics.cbegin(), n1.lyrics.cend(), n2.lyrics.cbegin(), [](const auto& a, const auto& b){ return a.text == b.text; }); }; };
        struct TieInfoCmp : F<decltype(&Self::tie_info), &Self::tie_info> {};
        struct DynamicCmp : F<decltype(&Self::dynamic), &Self::dynamic> {};
        struct ColorCmp : F<decltype(&Self::color), &Self::color> {};

        struct LengthCmp: F<decltype(&Self::length), &Self::length> {};
        struct StartCmp: F<decltype(&Self::start), &Self::start> {};
        struct EndCmp: F<decltype(&Self::end), &Self::end> {};

        using SimpleNoteAllowedComps = mpl::set<
                StartCmp,
                LengthCmp,
                EndCmp,
                PitchCmpExact,
                PitchCmpPs,
                LyricCmp,
                TieInfoCmp,
                DynamicCmp,
                ColorCmp>;

        template <typename ...Comps>
        bool _eq(const Self& other) const
        {
            using comparators_s = mpl::set<Comps...>;
            return k_eq<comparators_s>(other);
        }

        template <typename Set>
        bool k_eq(const Self& other) const
        {
            using comparators_s = Set;
            using comparators_well_defined = typename mpl::fold<
                    comparators_s,
                    mpl::true_,
                    mpl::and_<mpl::placeholders::_1, mpl::has_key<SimpleNoteAllowedComps, mpl::placeholders::_2>>
            >::type ;
            static_assert(comparators_well_defined::value, "Unknown comparator");

            struct and_ : And_Comps<Self>::template K<Set>{};
            return and_::cmp(*this, other);
        }

        bool exact_eq(const Self& other) const
        {
            return _eq<
                LengthCmp,
                StartCmp,
                EndCmp,
                PitchCmpExact,
                LyricCmp,
                TieInfoCmp,
                DynamicCmp,
                ColorCmp
                >(other);
        }

        bool ps_eq(SimpleNote<OffsetType, DurationType, PitchCont, LyricCont> const& other) const
        {
            return _eq<
                LengthCmp,
                StartCmp,
                EndCmp,
                PitchCmpPs,
                LyricCmp,
                TieInfoCmp,
                DynamicCmp,
                ColorCmp
            >(other);
        }

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(
                    cereal::base_class<Interval<OffsetType, DurationType>>(this),
                    CEREAL_NVP(pitches),
                    CEREAL_NVP(lyrics),
                    CEREAL_NVP(tie_info),
                    CEREAL_NVP(color),
                    CEREAL_NVP(dynamic));
        }
    };
}
#endif //AIDA_SIMPLENOTE_H
