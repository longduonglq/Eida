//
// Created by dmp on 1/22/22.
//

#ifndef AIDA_MEASURE_H
#define AIDA_MEASURE_H

#include <vector>
#include <algorithm>
#include <boost/range/numeric.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <numeric>

#include <cereal/cereal.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/vector.hpp>

#include "common/common.h"
#include "common/Interval.h"
#include "common/generator.hpp"
#include "GNote.h"

namespace m32 {
    template <
            typename OffsetT,
            typename DurationT,
            typename GNoteT,
            template < typename, typename... > class MeaCont,
            template < typename, typename... > class GNoteCont>
    class MeasuredPart;

    template <
            typename OffsetType,
            typename DurationType,
            typename GNoteT_ = GNote<OffsetType, DurationType>,
            template < typename, typename... > class GNoteCont = std::vector>
    class Measure : public Interval< OffsetType, DurationType > {
    public:
        using GNoteT = GNoteT_;
        using Self = Measure<OffsetType, DurationType, GNoteT, GNoteCont>;
        using OffsetT = OffsetType;
        using DurationT = DurationType;

        GNoteCont< GNoteT > gnotes;
        using MeasureNumberType = uint32_t;
        MeasureNumberType measure_number;
        std::optional<KeySigType> mea_key_sig;
        std::optional<ClefSignType> mea_clef_sign;
        std::optional<TimeSigType> mea_time_sig;

        Measure() = default;
        Measure(OffsetType offset,
                DurationType duration,
                MeasureNumberType measure_number,
                GNoteCont< GNoteT >&& gnotes)
                : Interval<OffsetType, DurationType>(offset, duration)
                , gnotes {std::move(gnotes)}
                , measure_number { measure_number }
                {}

        [[nodiscard]] DurationType get_elements_acc_duration() const
        {
            return std::reduce(
                    gnotes.cbegin(), gnotes.cend(),
                    DurationType{0, 1},
                    [](DurationType a, const auto& b) { return a + b.get_length(); }
                    );
        }

        [[nodiscard]] common::generator<typename GNoteT::SimpleNoteT const&>
        flat_const_iterator() const
        {
            for (const auto& gn : gnotes) {
                if (std::holds_alternative<typename GNoteT::TupletT>(gn)) {
                    for (const auto& sn : std::get<typename GNoteT::TupletT>(gn).notes) {
                        co_yield sn;
                    }
                }
                else co_yield std::get<typename GNoteT::SimpleNoteT>(gn);
            }
        }

        [[nodiscard]] bool is_rest_only() const
        {
            for (const auto& sn : flat_const_iterator()) {
                if (!sn.is_rest()) return false;
            }
            return true;
        }

        template <typename Keep>
        void set_start(OffsetType _start)
        {
            static_assert(std::is_same_v<Keep, KeepLength>, "keep-length only for now");
            DurationType displacement = _start - this->start;
            Interval<OffsetType, DurationType>::template set_start<Keep>(_start);
            for (auto& gnote: this->gnotes)
                std::visit(
                        [=](auto& gn) { gn.template displace_start<KeepLength>(displacement); },
                        gnote);
        }

        template <typename Keep>
        void displace_start(DurationType displacement)
        {
            static_assert(std::is_same_v<Keep, KeepLength>, "keep-length only for now");
            Interval<OffsetType, DurationType>::template displace_start<Keep>(displacement);
            for (auto& gnote: this->gnotes)
                std::visit(
                        [=](auto& gn) { gn.template displace_start<KeepLength>(displacement); },
                        gnote);
        }

        friend std::ostream& operator<<(std::ostream& os, const Measure& self)
        {
            std::ostringstream additional_attribs;
            if (self.mea_clef_sign.has_value()) additional_attribs << " | clef=" << self.mea_clef_sign.value();
            if (self.mea_key_sig.has_value()) additional_attribs << " | key=" << static_cast<int>(self.mea_key_sig.value());
            if (self.mea_time_sig.has_value()) additional_attribs << " | time=" << static_cast<int>(self.mea_time_sig.value().first) << "/" << static_cast<int>(self.mea_time_sig.value().second);

            os  << fmt::format("Measure<number={} | length={:.2f}{} | [{:.2f}, {:.2f})>--------",
                               self.measure_number, rational_to<float>(self.length), additional_attribs.str(),
                               rational_to<float>(self.start), rational_to<float>(self.end))
                << std::endl;

            for (const auto& currentGNote: self.gnotes)
                os << '\t' << currentGNote << std::endl;
            return os;
        }

        template <typename T, T f>
        using F = typename CmpField<Self>::template F<T, f>;

        struct MeaNumberCmp : F<decltype(&Self::measure_number), &Self::measure_number> {};
        struct MeaKeySigCmp : F<decltype(&Self::mea_key_sig), &Self::mea_key_sig> {};
        struct MeaClefSignCmp : F<decltype(&Self::mea_clef_sign), &Self::mea_clef_sign> {};
        struct MeaTimeSig : F<decltype(&Self::mea_time_sig), &Self::mea_time_sig> {};
        template <typename... HasherSet>
        struct HashCmp : F<decltype(&Self::template hash_self<HasherSet...>), &Self::template hash_self<HasherSet...>> {};

        using MeaAllowedComps_ = mpl::set<
                MeaNumberCmp,
                MeaKeySigCmp,
                MeaClefSignCmp,
                MeaTimeSig
        >;

        using MeaAllowedComps = mpl::joint_view<
                MeaAllowedComps_,
                typename GNoteT::GNoteAllowedComps
        >;

        template<typename... Comps>
        bool _eq (const Self& other) const {
            using comparators_s = mpl::set<Comps...>;
            return k_eq<comparators_s>(other);
        }

        template <typename Set>
        bool k_eq(const Self& other) const {
            using comparators_s = Set;
            using comparators_well_defined = typename mpl::fold<
                    comparators_s,
                    mpl::true_,
                    mpl::and_<
                            mpl::placeholders::_1,
                            mpl::contains<MeaAllowedComps, mpl::placeholders::_2>
                    >
            >::type;
            static_assert(comparators_well_defined::value, "Unknown comparators: Part");

            using mea_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::has_key<MeaAllowedComps_, mpl::placeholders::_1>
            >::type;
            using gn_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::contains<
                        typename GNoteT::GNoteAllowedComps,
                        mpl::placeholders::_1>
            >::type;

            struct mea_and_ : And_Comps<Self>::template K<mea_comparators_v> {};
            const auto inner_gnote_and_ = std::mem_fn(&GNoteT::template k_eq<gn_comparators_v>);
            return
                    mea_and_::cmp(*this, other) &&
                    std::equal(gnotes.cbegin(), gnotes.cend(), other.gnotes.cbegin(), inner_gnote_and_) ;
        }

        [[nodiscard]] bool exact_eq(const Self& other)
        {
            return _eq<
                MeaNumberCmp,
                MeaClefSignCmp,
                MeaKeySigCmp,
                MeaTimeSig,
                typename GNoteT::TupletT::NormalCmp,
                typename GNoteT::TupletT::ActualCmp,

                typename GNoteT::SimpleNoteT::PitchCmpExact,
                typename GNoteT::SimpleNoteT::LyricCmp,
                typename GNoteT::SimpleNoteT::TieInfoCmp,
                typename GNoteT::SimpleNoteT::DynamicCmp,
                typename GNoteT::SimpleNoteT::ColorCmp
            >(other);
        }

        template <typename Arch>
        void serialize(Arch& arch) {
            arch(
                    cereal::base_class<Interval<OffsetT, DurationT>>(this),
                    CEREAL_NVP(gnotes), CEREAL_NVP(measure_number), CEREAL_NVP(mea_time_sig), CEREAL_NVP(mea_clef_sign), CEREAL_NVP(mea_key_sig));
        }
    };
}

#endif //AIDA_MEASURE_H
