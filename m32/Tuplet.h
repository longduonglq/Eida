//
// Created by dop on 12/26/21.
//

#ifndef AIDA_TUPLET_H
#define AIDA_TUPLET_H

#include <cstdint>
#include <ostream>
#include <numeric>
#include <vector>
#include <algorithm>
#include <bit>

#include "common/Interval.h"
#include "common/gconfig.h"
#include "SimpleNote.h"
#include "utility.h"
#include "M32Except.h"
#include "Duration.h"
#include "GNote.h"

#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/contains.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

namespace m32 {
    class TupletException: public M32Exception {
    public:
        using M32Exception::M32Exception;
    };

    template<
            typename OffsetType,
            typename DurationType,
            typename SimpleNoteT = SimpleNote<OffsetType, DurationType>,
            template <typename, typename...> typename NoteCont = std::vector>
    requires IsDifferenceTypeOf<DurationType, OffsetType>
    class Tuplet : public Interval<OffsetType, DurationType> {
    public:
        using Self = Tuplet<OffsetType, DurationType, SimpleNoteT, NoteCont>;
        NoteCont< SimpleNoteT > notes;
        using NormalNumberType = uint16_t;
        NormalNumberType actual_number, normal_number;

        Tuplet() =default;
        Tuplet(
                NormalNumberType normal_num,
                NormalNumberType actual_num,
                OffsetType start,
                DurationType length,
                NoteCont< SimpleNoteT >&& simple_notes
                )
                : Interval<OffsetType, DurationType>(start, length)
                , normal_number {normal_num}
                , actual_number {actual_num}
                , notes {std::move(simple_notes)}
        {}

        void pack() {
            DurationType total_imaginary_length = std::reduce(
                    notes.cbegin(), notes.cend(),
                    DurationType{0},
                    [](const auto& a, const auto& b) { return a + b.length; });
            DurationType unit = this->length / total_imaginary_length;
            // rescaling
            if (notes.empty()) return;
            auto& front_note = notes.front();
            front_note.template set_start<KeepLength>(this->start);
            front_note.template set_length<KeepStart>(unit * notes.front().length);
            for (auto it = ++notes.begin(); it != notes.end(); it++) {
                it->template set_start<KeepLength>(std::prev(it)->end);
                it->template set_length<KeepStart>(unit * (it->length));
            }
        }

        bool is_packed() const {
            DurationType total_imaginary_length = std::reduce(
                    notes.cbegin(), notes.cend(),
                    DurationType{0},
                    [](const auto& a, const auto& b) { return a + b.length; });
            return total_imaginary_length == this->length;
        }

        template <typename Keep>
        void set_start(OffsetType _start)
        {
            static_assert(std::is_same_v<Keep, KeepLength>, "keep-length only for now");
            DurationType displacement = _start - this->start;
            Interval<OffsetType, DurationType>::template set_start<Keep>(_start);
            for (auto& gnote: notes)
                gnote.template displace_start<Keep>(displacement);
        }

        template <typename Keep>
        void displace_start(DurationType displacement)
        {
            static_assert(std::is_same_v<Keep, KeepLength>, "keep-length only for now");
            Interval<OffsetType, DurationType>::template displace_start<Keep>(displacement);
            for (auto& gnote: notes)
                gnote.template displace_start<Keep>(displacement);
        }

        template <typename DeltaType>
        void change_length_by(DeltaType delta)
        {
            /*
            * new_total_imaginary_length = new_tuplet_length/constant(unit);
            * new_nt_length
            * = (old_nt_length/old_total_imaginary_length) * new_total_imaginary_length
            * = (old_nt_length/old_total_imaginary_length) * new_tuplet_length/constant(unit)
            * = (old_nt_length/old_total_im_length) * new_tuplet_length * (old_total_im_length / old_tup_length)
            * = old_nt_length * (new_tuplet_length/old_tup_length)
            * */
            auto interval_length = Interval<OffsetType, DurationType>::length;
            const auto new_tuplet_length = interval_length + delta;
            const auto scaling_factor = new_tuplet_length / interval_length;
            Interval<OffsetType, DurationType>::template change_length_by<KeepStart>(delta);
            for (auto& gnote: notes)
                gnote.template scale_length_by<KeepStart>(scaling_factor);

            for (auto it = ++notes.begin(); it != notes.end(); it++)
                it->template set_start<KeepLength>(std::prev(it)->end);
        }

        [[nodiscard]]
        NoteCont<SimpleNoteT> flatten() const
        {
            if (!is_packed()) throw m32::TupletException("Are you sure you want to flatten an unpacked tuple? The tuple is: \n {}", *this);

            NoteCont<SimpleNoteT> flat;
            auto unit = this->length / actual_number;
            DurationType total_imaginary_length = std::reduce(
                    notes.cbegin(), notes.cend(),
                    DurationType{0},
                    [](const auto& a, const auto& b) { return a + b.length; });

            auto last_offset = this->start;
            for (const auto& note: notes)
            {
                auto new_nt = note;
                new_nt.template set_start<KeepLength>(last_offset);
                auto new_length = note.length / unit;
                new_nt.template set_length<KeepStart>(new_length);
                last_offset += new_length;

                flat.push_back(new_nt);
            }

            return flat;
        }

        // this method leaves this->normal_number untouched
        template <bool CreateTie = true>
        std::pair< GNote< OffsetType, DurationType>, GNote<OffsetType, DurationType> >
        split_at_offset(OffsetType offset) const
        {
            if (!is_packed()) throw m32::TupletException("Are you sure you want to flatten an unpacked tuple? The tuple is: \n {}", *this);

            if (!std::has_single_bit(
                    static_cast<std::make_unsigned_t<typename decltype(offset)::int_type>>
                            (offset.denominator())))
                throw m32::TupletException("Can't split at offset whose denominator is not a power of 2");

            if (offset < this->start || this->end < offset)
                throw m32::TupletException("Can't split at offset that is outside of the tuplet's interval");

            const static auto frac_to_int = []<typename Int=int>
                    (const boost::rational<Int>& f)
            {
                if (f.denominator() == 1) return f.numerator();
                else return f.denominator() * f.numerator(); // assuming boost::rational keeps fraction in reduced state
            };

            // shift interval's start to 0 for ease since offset ~ length then reverse at the end
            auto start_backup = this->start;
            Tuplet<OffsetType, DurationType> self = *this;
            self.template displace_start<KeepLength>(-start_backup);
            offset -= start_backup;

            auto orig_tup_unit = self.length / self.actual_number;
            decltype(actual_number) N = 0; // = how many tup_unit can fit in the first segment [0, offset)
            for (; N <= actual_number; N++) {
                if (offset - N * orig_tup_unit < orig_tup_unit) break;
            }

            auto left_unit = N < 1 ? offset - N * orig_tup_unit : m32::frac_gcd(offset - N * orig_tup_unit, orig_tup_unit);
            auto right_unit = (actual_number - N == 1) ? (N + 1) * orig_tup_unit - offset : m32::frac_gcd((N + 1) * orig_tup_unit - offset, orig_tup_unit);

            // left half
            GNote<OffsetType, DurationType> left_part = self; // will get reassigned anyway
            if (N < 1) {
                SimpleNoteT new_nt = self.notes.front();
                new_nt.template set_length<KeepStart>(left_unit);
                if constexpr (CreateTie) new_nt.tie_info |= TieInfo::TieStart;
                left_part = std::move(new_nt);
            }
            else if (N >= 1) {
                auto left_actual_number = frac_to_int(offset / left_unit);
                auto left_detupletized_unit_length = m32::duration_st<m32::Infimum, m32::Closed>(offset / left_actual_number).value(); // The `detupletized` length of the tuplet unit
                decltype(normal_number) left_normal_number = frac_to_int(offset / left_detupletized_unit_length);
                auto left_tup = Tuplet<OffsetType, DurationType>(
                        left_normal_number,
                        left_actual_number,
                        OffsetType{0},
                        static_cast<DurationType>(offset),
                        {});
                left_tup.template set_length<KeepStart>(offset);

                for (const auto& sn : self.notes) {
                    if (sn.start > offset) break;
                    left_tup.notes.push_back(sn);
                }
                left_tup.notes.back().template set_length<KeepStart>(left_unit);
                left_tup.notes.back().tie_info |= TieInfo::TieStart;
                left_part = std::move(left_tup);
            }
            else assert(false && "unreachable");

            // right half
            GNote<OffsetType, DurationType> right_part = self;
            if (actual_number - N == 1) {
                SimpleNoteT new_nt = self.notes.back();
                new_nt.template set_length<KeepStart>(right_unit);
                new_nt.template set_start<KeepLength>(offset);
                new_nt.tie_info |= TieInfo::TieEnd;
                right_part = std::move(new_nt);
            }
            else if (actual_number - N > 1) {
                auto r_length = self.end - offset;
                auto right_actual_number = frac_to_int(r_length / right_unit);
                auto right_detupletized_unit_length = m32::duration_st<m32::Infimum, m32::Closed>(r_length / right_actual_number).value();
                decltype(normal_number) right_normal_number = frac_to_int(r_length / right_detupletized_unit_length);
                auto right_tup = Tuplet<OffsetType, DurationType>(
                        right_normal_number,
                        right_actual_number,
                        offset,
                        static_cast<DurationType>(r_length),
                        {});
                right_tup.template set_length<KeepStart>(self.length - offset);

                for (const auto& sn : self.notes) {
                    if (sn.end <= offset) continue;
                    right_tup.notes.push_back(sn);
                }
                right_tup.notes.front().template set_start<KeepEnd>(offset);
                right_tup.notes.front().tie_info |= TieInfo::TieEnd;
                right_part = std::move(right_tup);
            }
            else assert(false && "unreachable");

            // shifted interval back to normal
            // self.template displace_start<KeepLength>(+start_backup);
            std::visit([start_backup](auto& l, auto& r){
                l.template displace_start<KeepLength>(+start_backup);
                r.template displace_start<KeepLength>(+start_backup);
            }, left_part, right_part);
            return {left_part, right_part};
        }

        std::pair< GNote< OffsetType, DurationType>, GNote<OffsetType, DurationType> >
        split_and_sever_ties_at_offset(OffsetType offset) const {
            auto [left, right] = split_at_offset(offset);
            if (left.is_simple_note()) left.as_simple_note().tie_info &= ~TieInfo::TieStart;
            else left.as_tuplet().notes.back().tie_info &= ~TieInfo::TieStart;

            if (right.is_simple_note()) right.as_simple_note().tie_info &= ~TieInfo::TieStart;
            else right.as_tuplet().notes.front().tie_info &= ~TieInfo::TieEnd;
            return std::make_pair(left, right);
        }

        template<typename T, T f>
        using F = typename CmpField<Self>::template F<T, f>;
        struct NormalCmp : F<decltype(&Self::normal_number), &Self::normal_number> {};
        struct ActualCmp : F<decltype(&Self::actual_number), &Self::actual_number> {};
        struct NormalActualCmp : And_Comps<Self>::template F<NormalCmp, ActualCmp>{};
        struct LengthCmp: F<decltype(&Self::length), &Self::length> {};
        struct StartCmp: F<decltype(&Self::start), &Self::start> {};
        struct EndCmp: F<decltype(&Self::end), &Self::end> {};

        using TupletAllowedComps_ = mpl::set<
                StartCmp,
                LengthCmp,
                EndCmp,
                NormalCmp,
                ActualCmp,
                NormalActualCmp>;
        using TupletAllowedComps = mpl::joint_view<
                TupletAllowedComps_,
                typename SimpleNoteT::SimpleNoteAllowedComps
                >;

        template <typename ... Comps>
        bool _eq(const Self& other) const {
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
                            mpl::contains<TupletAllowedComps, mpl::placeholders::_2>
                    >
            >::type;
            static_assert(comparators_well_defined::value, "Unknown comparators: Tuplet");
            using tup_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::contains<TupletAllowedComps_, mpl::placeholders::_1>
            >::type;
            using sn_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::contains<typename SimpleNoteT::SimpleNoteAllowedComps, mpl::placeholders::_1>
            >::type;

            struct tup_and_ : And_Comps<Self>::template K<tup_comparators_v> {};
            const auto inner_sn_and_ = std::mem_fn(&SimpleNoteT::template k_eq<sn_comparators_v>);
            return
                tup_and_::cmp(*this, other) &&
                notes.size() == other.notes.size() &&
                std::equal(notes.cbegin(), notes.cend(), other.notes.cbegin(), inner_sn_and_);
        }

        bool exact_eq(const Self& other) const
        {
            return _eq<
                StartCmp,
                EndCmp,
                LengthCmp,
                NormalActualCmp,
                typename SimpleNoteT::StartCmp,
                typename SimpleNoteT::PitchCmpExact,
                typename SimpleNoteT::LyricCmp,
                typename SimpleNoteT::TieInfoCmp,
                typename SimpleNoteT::DynamicCmp,
                typename SimpleNoteT::ColorCmp
            >(other);
        }

        bool is_tuplet() const { return true; }
        bool is_simple_note() const { return false; }

        friend std::ostream& operator<< (std::ostream& os, const Tuplet& self)
        {
            os  << '\t'
                << fmt::format("Tuplet<[{:.2f}, {:.2f}) | normal={}, actual={}",
                               m32::rational_to<float>(self.start),
                               m32::rational_to<float>(self.end),
                               self.normal_number,
                               self.actual_number)
                << std::endl;
            for (const auto& simpleNote: self.notes)
            {
                os << "\t(@tup)" << simpleNote << std::endl;
            }
            os << ">" << std::endl;
            return os;
        }

        template <typename Arch>
        void serialize(Arch& arch) {
            arch(
                    cereal::base_class<Interval<OffsetType, DurationType>>(this),
                    CEREAL_NVP(actual_number),
                    CEREAL_NVP(normal_number),
                    CEREAL_NVP(notes)
            );
        }
    };
}

template <
        typename OffsetT,
        typename DurationT,
        typename SimpleNoteT,
        template <typename, typename ...> typename NoteCont>
struct fmt::formatter<m32::Tuplet<OffsetT, DurationT, SimpleNoteT, NoteCont>>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const m32::Tuplet<OffsetT, DurationT, SimpleNoteT, NoteCont>& tup, FormatContext& ctx) -> decltype(ctx.out())
    {
        std::stringstream ss;
        ss << tup << std::endl;
        return fmt::format_to(ctx.out(), "{}", ss.str());
    }
};

#endif //AIDA_TUPLET_H