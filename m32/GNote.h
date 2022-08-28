// Project AIDA
// Created by Long Duong on 12/21/21.
// Purpose: 
//

#ifndef AIDA_GNOTE_H
#define AIDA_GNOTE_H

#include <vector>
#include <variant>
#include <algorithm>
#include <functional>
#include <boost/range/adaptors.hpp>
#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/contains.hpp>
#include <common/generator.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>

#include "cmps.h"
#include "SimpleNote.h"

namespace m32 {
    template<
            typename OffsetType,
            typename DurationType,
            typename SimpleNoteT,
            template <typename, typename...> typename NoteCont>
    requires IsDifferenceTypeOf<DurationType, OffsetType>
    class Tuplet;

    template<
            typename OffsetType,
            typename DurationType,
            template<typename, typename...> typename PitchCont = std::vector,
            template<typename, typename...> typename LyricCont = PitchCont,
            template<typename, typename...> typename TupNoteCont = PitchCont>
    using VariantGNote = std::variant<
        SimpleNote<OffsetType, DurationType, PitchCont, LyricCont>,
        Tuplet<
            OffsetType,
            DurationType,
            SimpleNote<OffsetType, DurationType, PitchCont, LyricCont>,
            TupNoteCont>
        >;

    template<
            typename OffsetType,
            typename DurationType,
            template <typename, typename...> typename PitchCont = std::vector,
            template <typename, typename...> typename LyricCont = PitchCont,
            template <typename, typename...> typename TupNoteCont = PitchCont>
    class GNote : public VariantGNote<OffsetType, DurationType, PitchCont, LyricCont, TupNoteCont>
    {
    public:
        // using the base's constructor
        using VariantGNote<OffsetType, DurationType, PitchCont, LyricCont>::variant::variant;
        using SimpleNoteT = SimpleNote<
                OffsetType,
                DurationType,
                PitchCont,
                LyricCont >;
        using TupletT = Tuplet<
                OffsetType,
                DurationType,
                SimpleNote <OffsetType, DurationType, PitchCont, LyricCont>,
                TupNoteCont>;
        using Self = GNote<OffsetType, DurationType, PitchCont, LyricCont, TupNoteCont>;
        using OffsetT = OffsetType;
        using DurationT = DurationType;
        using IntervalT = typename SimpleNoteT::Interval;

        // Interval stuff
        OffsetType get_start() const { return const_interval().start; }
        OffsetType get_end() const { return const_interval().end; }
        OffsetType get_length() const { return const_interval().length; }
        IntervalT const_interval() const { return std::visit([](auto& self){ return IntervalT(self.start, self.length); }, *this); }

        template <typename Keep>
        void displace_start(DurationType displacement)
        {
            return std::visit([=](auto& self){ return self.template displace_start<Keep>(displacement);}, *this);
        }

        template <typename Keep>
        void set_start(OffsetType start)
        {
            return std::visit([=](auto& self){ return self.template set_start<Keep>(start);}, *this);
        }
        template <typename Keep>
        void set_end(OffsetType start)
        {
            return std::visit([=](auto& self){ return self.template set_end<Keep>(start);}, *this);
        }

        bool is_tuplet() const { return std::holds_alternative<TupletT>(*this); }
        bool is_simple_note() const { return std::holds_alternative<SimpleNoteT>(*this); }
        const SimpleNoteT& as_const_simple_note() const { return std::get<SimpleNoteT>(*this); }
        const TupletT& as_const_tuplet() const { return std::get<TupletT>(*this); }
        SimpleNoteT& as_simple_note() { return std::get<SimpleNoteT>(*this); }
        TupletT& as_tuplet() { return std::get<TupletT>(*this); }

        void pack_if_tuplet() {
            if (std::holds_alternative<TupletT>(*this)) {
                auto& t = std::get<TupletT>(*this);
                if (!t.is_packed()) return t.pack();
            }
        }

        std::pair< GNote< OffsetType, DurationType>, GNote<OffsetType, DurationType> >
        split_at_offset(OffsetT offset) const
        {
            if (is_tuplet()) return as_const_tuplet().split_at_offset(offset);
            else {
                auto [first, sec] = as_const_simple_note().split_at_offset(offset);
                return std::make_pair(
                        GNote< OffsetType, DurationType >(first),
                        GNote< OffsetType, DurationType >(sec)
                        );
            }
        }

        std::pair< GNote< OffsetType, DurationType>, GNote<OffsetType, DurationType> >
        split_and_sever_ties_at_offset(OffsetT offset) const
        {
            if (is_tuplet()) return as_tuplet().split_and_sever_ties_at_offset(offset);
            else return as_simple_note().split_and_sever_ties_at_offset(offset);
        }

        template <typename T, T f>
        using F = typename CmpField<Self>::template F<T, f>;
        struct LengthCmp: F<decltype(&Self::get_length), &Self::get_length> {};
        struct StartCmp: F<decltype(&Self::get_start), &Self::get_start> {};
        struct EndCmp: F<decltype(&Self::get_end), &Self::get_end> {};
        using GNoteAllowedComps_ = mpl::set<
                StartCmp,
                LengthCmp,
                EndCmp
                >;

        using GNoteAllowedComps = mpl::joint_view<
                GNoteAllowedComps_,
                mpl::joint_view<
                    typename Self::SimpleNoteT::SimpleNoteAllowedComps,
                    typename Self::TupletT::TupletAllowedComps
                    >
                >;

        template <typename... Comps>
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
                    mpl::and_<
                            mpl::placeholders::_1,
                            mpl::contains<GNoteAllowedComps, mpl::placeholders::_2>
                    >
            >::type;
            static_assert(comparators_well_defined::value, "Unknown comparator");
            using gnote_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::contains<GNoteAllowedComps_, mpl::placeholders::_1>
            >::type;
            using tup_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::contains<typename TupletT::TupletAllowedComps, mpl::placeholders::_1>
            >::type;
            using sn_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::contains<typename SimpleNoteT::SimpleNoteAllowedComps, mpl::placeholders::_1>
            >::type;

            struct gn_and_ : And_Comps<Self>::template K<gnote_comparators_v> {};
            constexpr auto tup_eq_ = std::mem_fn(&TupletT::template k_eq<typename mpl::joint_view<tup_comparators_v, sn_comparators_v>::type>);
            constexpr auto sn_eq_ = std::mem_fn(&SimpleNoteT::template k_eq<sn_comparators_v>);

            return
                gn_and_::cmp(*this, other) &&
                std::visit([tup_eq_, sn_eq_](auto& self, auto& nt) {
                    if constexpr(!std::is_same_v<
                            std::remove_cvref_t<decltype(self)>,
                            std::remove_cvref_t<decltype(nt)>>) { return false; }
                    else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(self)>, SimpleNoteT>) { return sn_eq_(self, nt); }
                    else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(self)>, TupletT>) { return tup_eq_(self, nt); }
                }, *this, other);
        }

        [[nodiscard]] bool exact_eq(const GNote<OffsetType, DurationType, PitchCont, LyricCont, TupNoteCont>& other) const
        {
            return _eq<
                StartCmp,
                LengthCmp,
                EndCmp,

                typename TupletT::NormalCmp,
                typename TupletT::ActualCmp,

                typename SimpleNoteT::PitchCmpExact,
                typename SimpleNoteT::LyricCmp,
                typename SimpleNoteT::TieInfoCmp,
                typename SimpleNoteT::DynamicCmp,
                typename SimpleNoteT::ColorCmp
                >(other);
        }

        friend std::ostream& operator<<(std::ostream& os, const GNote& self)
        {
            std::visit(
                    [&os](const auto& gnote) { os << gnote ;},
                    self
            )  ;
            return os;
        }

        template <typename Arch>
        void serialize(Arch& arch, Self& tup) {
            arch(cereal::make_nvp("GNote", *this));
        }
    };
}

#endif //AIDA_GNOTE_H
