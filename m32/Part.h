//
// Created by dmp on 1/22/22.
//

#ifndef AIDA_PART_H
#define AIDA_PART_H

#include <utility>
#include <vector>
#include <string>
#include <memory>
#include <list>
#include <deque>
#include <algorithm>
#include <iostream>

#include <boost/mpl/sort.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/less.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/mpl/has_key.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/range.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "cmps.h"

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>

#include <common/gconfig.h>
#include <common/generator.hpp>
#include <botan/hash.h>
#include <botan/hex.h>

#include "m32/types.h"
#include "m32/Measure.h"
#include "m32/GNote.h"

namespace m32
{
    struct GetOrder {
        template <typename T, typename U>
        struct apply{ using type = typename boost::mpl::less<typename T::id, typename U::id>::type; };
    };

    template <typename PartT, typename GNoteT>
    struct NoteHasherMixin {
        struct HashPitch {
            using id = boost::mpl::size_t<0>;
            template <typename Hasher >
            static void contribute(Hasher& hasher, const typename GNoteT::SimpleNoteT& simple_note)
            {
                if (!simple_note.is_rest()) {
                    for (const auto& pitch: simple_note.pitches) {
                        // TODO: check the memory safety of this
                        hasher.update(
                                reinterpret_cast<const uint8_t*>(&pitch.ps),
                                sizeof(decltype(pitch.ps)) / sizeof(uint8_t));
                    }
                }
                else hasher.update('R');
            }
        };

        struct HashDuration {
            using id = boost::mpl::size_t<1>;
            template <typename Hasher >
            static void contribute(Hasher& hasher, const typename GNoteT::SimpleNoteT& simple_note)
            {
                // Hash Duration
                hasher.update(
                        reinterpret_cast<const uint8_t*>(&simple_note.start.numerator()),
                        sizeof(decltype(simple_note.start.numerator())) / sizeof(uint8_t)
                );
                hasher.update(
                        reinterpret_cast<const uint8_t*>(&simple_note.start.denominator()),
                        sizeof(decltype(simple_note.start.denominator())) / sizeof(uint8_t)
                );
                hasher.update(
                        reinterpret_cast<const uint8_t*>(&simple_note.end.numerator()),
                        sizeof(decltype(simple_note.end.numerator())) / sizeof(uint8_t)
                );
                hasher.update(
                        reinterpret_cast<const uint8_t*>(&simple_note.end.denominator()),
                        sizeof(decltype(simple_note.end.denominator())) / sizeof(uint8_t)
                );
            }
        };

        struct HashLyrics {
            using id = boost::mpl::size_t<2>;
            template <typename Hasher >
            static void contribute(Hasher& hasher, const typename GNoteT::SimpleNoteT& simple_note)
            {
                for (const auto& lyric: simple_note.lyrics) { hasher.update(lyric.text); }
            }
        };

        template <typename SNt>
        struct NoteApplyContribute {
            Botan::HashFunction* hash_func = nullptr;
            const SNt* simple_note = nullptr;

            template <typename Hasher>
            void operator()(Hasher) { Hasher::contribute(*hash_func, *simple_note); }
        };

        using NoteHashersSet = mpl::set<
                HashPitch,
                HashDuration,
                HashLyrics
        >;

        template <typename HasherSet >
        void k_hash_notes() const {
            static_assert(mpl::is_sequence<HasherSet>::value);

            if constexpr(std::is_same_v<typename mpl::less<mpl::size<HasherSet>, mpl::size_t<1>>::type, mpl::true_>) {
                return ;
            }
            else {
                static_cast<const PartT&>(*this).init_sha();
                using note_hashers_welldef = typename mpl::fold<
                        HasherSet,
                        mpl::true_,
                        mpl::and_<
                                mpl::placeholders::_1,
                                mpl::has_key<NoteHashersSet, mpl::placeholders::_2>>
                >::type;

                static_assert(note_hashers_welldef::value, "Unknown hasher: PartNote");
                using pred = typename boost::mpl::lambda<GetOrder>::type;
                using inserter = boost::mpl::back_inserter<boost::mpl::vector<> >;
                using sorted_note_hashers = typename mpl::sort<HasherSet, pred, inserter>::type;

                for (const auto &simple_note: static_cast<const PartT &>(*this).simple_note_iter()) {
                    mpl::for_each<sorted_note_hashers>(
                        NoteApplyContribute<typename GNoteT::SimpleNoteT>{
                            static_cast<const PartT&>(*this).sha.get(), &simple_note
                        });
                }
            }
        }

        template< typename... Hashers >
        void hash_notes() const
        {
            static_assert(!mpl::is_sequence<Hashers...>::value);
            using hashers_s = mpl::set<Hashers...>;
            return k_hash_notes<hashers_s>();
        }
    };

    template <typename PartT, typename GNoteT>
    struct PartInfoHasherMixin {
        struct HashClef {
            using id = boost::mpl::size_t<0>;
            template <typename Hs>
            static void contribute(Hs& hs, const PartT& part) { hs.update(static_cast<uint32_t> (part.clef_sign)); }
        };

        struct HashKeySig {
            using id = boost::mpl::size_t<1>;
            template <typename Hs>
            static void contribute(Hs& hs, const PartT& part) { hs.update_be(static_cast<uint32_t> (part.key_sig)); }
        };

        struct HashTimeSig {
            using id = boost::mpl::size_t<2>;
            template <typename Hs>
            static void contribute(Hs& hs, const PartT& part) {
                hs.update_be(static_cast<uint32_t> (part.time_sig.first));
                hs.update_be(static_cast<uint32_t> (part.time_sig.second));
            }
        };

        using PartHashersSet = mpl::set<
                HashClef,
                HashKeySig,
                HashTimeSig
        >;

        struct PartApplyContribute {
            Botan::HashFunction* hash_func = nullptr;
            const PartT* part = nullptr;

            template <typename Hasher>
            void operator()(Hasher) { Hasher::contribute(*hash_func, *part); }
        };

        template <typename HasherSet >
        void k_hash_part() const {
            static_assert(mpl::is_sequence<HasherSet>::value);

            if constexpr(std::is_same_v<typename mpl::less<mpl::size<HasherSet>, mpl::size_t<1>>::type, mpl::true_>) {
                return;
            }
            else {
                static_cast<const PartT&>(*this).init_sha();

                using part_hashers_welldef = typename mpl::fold<
                    PartHashersSet,
                    mpl::true_,
                    mpl::and_<
                        mpl::placeholders::_1,
                        mpl::has_key<PartHashersSet, mpl::placeholders::_2>>
                >::type;

                using pred = typename boost::mpl::lambda<GetOrder>::type;
                using inserter = boost::mpl::back_inserter<boost::mpl::vector<> >;
                using sorted_part_hashers = typename mpl::sort<HasherSet, pred, inserter>::type;

                boost::mpl::for_each<sorted_part_hashers>(PartApplyContribute{static_cast<const PartT&>(*this).sha.get(), static_cast<const PartT*>(this)});
            }
        }

        template< typename... Hashers >
        void hash_part() const
        {
            static_assert(!mpl::is_sequence<Hashers...>::value);
            using hashers_s = mpl::set<Hashers...>;
            return k_hash_part<hashers_s>();
        }
    };

    template <typename PartT, typename GNoteT>
    struct HashSelfMixin {
        template <typename NoteHashers, typename PartHashers>
        Botan::secure_vector<uint8_t> k_hash_self() const {
            if constexpr(
                std::is_same_v<
                    typename mpl::less<
                        mpl::plus<mpl::size<NoteHashers>, mpl::size<PartHashers>>, mpl::size_t<1>
                        >::type,
                    mpl::true_>){ return {} ; }
            else {
                static_cast<const PartT&>(*this).init_sha();
                static_cast<const PartT&>(*this).NoteHasherMixin<PartT, GNoteT>::template k_hash_notes<NoteHashers>();
                static_cast<const PartT&>(*this).PartInfoHasherMixin<PartT, GNoteT>::template k_hash_part<PartHashers>();
                return static_cast<const PartT&>(*this).sha->final();
            }
        }

        template <typename... Hashers>
        Botan::secure_vector<uint8_t> hash_self() const {
            using hashers_s = mpl::set<Hashers...>;
            using part_hashers_v = typename mpl::filter_view<
                    hashers_s,
                    mpl::has_key<typename PartInfoHasherMixin<PartT, GNoteT>::PartHashersSet, mpl::placeholders::_>
            >::type;
            using note_hashers_v = typename mpl::filter_view<
                    hashers_s,
                    mpl::has_key<typename NoteHasherMixin<PartT, GNoteT>::NoteHashersSet , mpl::placeholders::_>
            >::type;
            return k_hash_self<note_hashers_v, part_hashers_v>();
        }
    };

    template <
            typename OffsetT,
            typename DurationT,
            typename GNoteT,
            template < typename, typename... > class MeaCont = std::vector,
            template < typename, typename... > class GNoteCont = std::vector>
    class MeasuredPart;

    template <
            typename GNoteT_,
            template < typename, typename... > class GNoteCont = std::vector>
    class Part
            : public NoteHasherMixin<Part<GNoteT_, GNoteCont>, GNoteT_>
            , public PartInfoHasherMixin<Part<GNoteT_, GNoteCont>, GNoteT_>
            , public HashSelfMixin<Part<GNoteT_, GNoteCont>, GNoteT_>
    {
    public:
        std::string part_name;
        KeySigType key_sig;
        ClefSignType clef_sign;
        TimeSigType time_sig;

        using GNoteT = GNoteT_;
        GNoteCont< GNoteT > gnotes;
        mutable std::unique_ptr<Botan::HashFunction> sha;
        using Self = Part<GNoteT, GNoteCont>;
        using SimpleNoteT = typename GNoteT::SimpleNoteT;
        using TupletT = typename GNoteT::TupletT;
        using OffsetT = typename GNoteT::OffsetT;

        Part() = default;
        Part(std::string part_name,
             KeySigType key_sig,
             ClefSignType clef_sign,
             TimeSigType time_sig)
             : part_name {std::move(part_name)}
             , key_sig {key_sig}
             , clef_sign {clef_sign}
             , time_sig {std::move(time_sig)}
             , sha { nullptr }
        {}
        explicit Part(GNoteCont< GNoteT >&& gnotes)
            : gnotes { std::move(gnotes) }
        {}

        Part(const Self& other)
            : part_name {other.part_name}
            , key_sig {other.key_sig}
            , clef_sign {other.clef_sign}
            , time_sig {other.time_sig}
            , gnotes { other.gnotes }
            , sha { nullptr }
        {}

        template <typename Arch>
        void serialize(Arch& arch) {
            arch(
                    CEREAL_NVP(part_name),
                    CEREAL_NVP(key_sig),
                    CEREAL_NVP(clef_sign),
                    CEREAL_NVP(time_sig),
                    CEREAL_NVP(gnotes)
                    );
        }

        void init_sha() const { if (!sha) sha = std::unique_ptr<Botan::HashFunction>(Botan::HashFunction::create("SHA-512")); }

        MeasuredPart<
            typename GNoteT::OffsetT,
            typename GNoteT::DurationT,
            GNoteT,
            GNoteCont>
        to_measured() const
        {
            auto measured_part = MeasuredPart<
                    typename GNoteT::OffsetT,
                    typename GNoteT::DurationT,
                    GNoteT,
                    GNoteCont> (part_name, key_sig, clef_sign, time_sig);
            using MeasureT = typename decltype(measured_part)::MeasureT;
            if (gnotes.empty()) return measured_part;

            // Measure length of timeSig a/b is given by a * (4 / b)
            using DurationT = typename GNoteT::SimpleNoteT::DurationT;
            auto measure_length = DurationT{ time_sig.first * 4, time_sig.second };
            auto est_number_of_measures = 1 + rational_to<typename MeasureT::MeasureNumberType>(gnotes.back().get_end() / measure_length);
            measured_part.measures.reserve(est_number_of_measures);

            measured_part.measures.emplace_back(
                    typename GNoteT::OffsetT{0, 1},
                    measure_length,
                    static_cast<typename MeasureT::MeasureNumberType>(0),
                    GNoteCont< GNoteT >());

            std::list<GNoteT> _tgnotes (gnotes.cbegin(), gnotes.cend());
            auto cur_note = _tgnotes.begin();
            while (cur_note != _tgnotes.end())
            {
                cur_note->pack_if_tuplet();
                auto& current_measure = measured_part.measures.back();
                if (current_measure.does_swallow(cur_note->const_interval())) {
                    cur_note->template displace_start<KeepLength>(-current_measure.start);
                    current_measure.gnotes.push_back(std::move(*cur_note));
                    std::advance(cur_note, 1);
                }
                else if (current_measure.end == cur_note->get_start()) {
                    measured_part.measures.emplace_back(
                            current_measure.end,
                            measure_length,
                            current_measure.measure_number + 1,
                            GNoteCont<GNoteT>()
                            );
                    auto& updated_current = measured_part.measures.back();
                    cur_note->template displace_start<KeepLength>(-updated_current.start);
                    updated_current.gnotes.push_back(std::move(*cur_note));
                    std::advance(cur_note, 1);
                }
                else if (current_measure.does_overlap_with(cur_note->const_interval()))
                {
                    auto [left, right] = cur_note->split_at_offset(current_measure.end);

                    // shift offset
                    left.template displace_start<KeepLength>(-current_measure.start);
                    current_measure.gnotes.push_back(std::move(left));

                    measured_part.measures.emplace_back(
                            current_measure.end,
                            measure_length,
                            current_measure.measure_number + 1,
                            GNoteCont< GNoteT >());

                    auto it_after = _tgnotes.erase(cur_note);
                    cur_note = _tgnotes.insert(it_after, std::move(right));
                }
                else assert(false && "Unreachable");
            }

            assert(std::all_of(measured_part.measures.cbegin(), std::prev(measured_part.measures.cend()), [](const MeasureT& measure) { return measure.get_elements_acc_duration() == measure.length; } ));
            return measured_part;
        }

        // this function assumes iterators are stable in memory
        template <typename ...T>
        using StableCont = std::list<T...>;
        void fuse_tail_(StableCont<GNoteT>& ran, typename StableCont<GNoteT>::iterator iter) {
            const static auto are_fusable = [](const auto& n1, const auto& n2) {
                return
                    n1.pitches.size() == n2.pitches.size() &&
                    std::equal(n1.pitches.cbegin(), n1.pitches.cend(), n2.pitches.cbegin(), [](auto &a, auto& b) { return a.exact_eq(b); }) &&
                    n1.end == n2.start;
            };
            const static auto combine_tie = [](TieInfo t1, TieInfo t2) -> TieInfo {
                if ((t1 & TieInfo::TieStart) && (t2 & TieInfo::TieEnd)) {
                    t1 &= TieInfo(~TieInfo::TieStart);
                    t2 &= TieInfo(~TieInfo::TieEnd);
                }
                return t1 | t2;
            };

            if (iter == ran.end()) return;

            if (iter->is_tuplet()) {
                auto& tup = iter->as_tuplet();
                StableCont<GNoteT> _gnotes(tup.notes.begin(), tup.notes.end());
                fuse_tail_(_gnotes, _gnotes.begin());

                tup.notes.clear();
                const static auto convert = [](auto it) { return std::get<typename GNoteT::SimpleNoteT> (it); };
                tup.notes.insert(
                        tup.notes.end(),
                        boost::make_transform_iterator(_gnotes.begin(), convert),
                        boost::make_transform_iterator(_gnotes.end(), convert));
                fuse_tail_(ran, std::next(iter));
            }
            else if (iter->is_simple_note()) {
                fuse_tail_(ran, std::next(iter));
                typename GNoteT::SimpleNoteT& sn = iter->as_simple_note();
                if (sn.tie_info & TieInfo::TieStart &&
                    std::next(iter) != ran.end() &&
                    // determine fusability
                    std::holds_alternative<typename GNoteT::SimpleNoteT>(*std::next(iter)) &&
                    are_fusable(sn, std::get<typename GNoteT::SimpleNoteT>(*std::next(iter)))
                    ) {
                    if (!std::next(iter)->is_tuplet()) // don't fuse outside note with in-tuplet
                    {
                        sn.tie_info = combine_tie(sn.tie_info, std::next(iter)->as_simple_note().tie_info);
                        sn.merge_with(std::next(iter)->as_simple_note());
                        ran.erase(std::next(iter));
                    }
                }
            }
            else assert(false);
        }

        Part fuse_tied_notes() {
            Part<GNoteT, GNoteCont> new_part(*this);
            StableCont<GNoteT> _gnotes(new_part.gnotes.begin(), new_part.gnotes.end());

            fuse_tail_(_gnotes, _gnotes.begin());

            new_part.gnotes.clear();
            new_part.gnotes.insert(
                    new_part.gnotes.end(),
                    std::make_move_iterator(_gnotes.begin()),
                    std::make_move_iterator(_gnotes.end()));
            return new_part;
        }

        struct CreateTies{};
        struct SeverTies{};
        // assumes offsets are sorted in increasing order
        // assumes that gnote vector is properly reserved so as not to introduce any allocation.
        template <typename TieTreatment, typename GNoteIt, typename OffsetIt>
        void split_at_offsets_helper(
                GNoteIt current_gnote,
                GNoteIt end_gnote,
                OffsetIt current_offset,
                OffsetIt end_offset,
                int displacement)
        {
            if (current_gnote == end_gnote && current_offset == end_offset) {
                // Caller is responsible for making sure that this assignment is valid
                *(current_gnote + displacement) = *current_gnote;
                return;
            }
            else if (current_gnote == end_gnote && current_offset != end_offset) {
                *(current_gnote + displacement) = *current_gnote;
                return;
            }
            else if (current_gnote != end_gnote && current_offset == end_offset ) {
                split_at_offsets_helper<TieTreatment>(
                        current_gnote + 1,
                        end_gnote,
                        current_offset,
                        end_offset,
                        displacement
                        );
                return;
            }
            else { // current_gnote != end_gnote && current_offset != end_offset
                while (
                    current_offset != end_offset &&
                    *current_offset < current_gnote->get_start())
                {
                    current_offset++;
                }

                if (current_offset == end_offset)
                    split_at_offsets_helper<TieTreatment>(
                            current_gnote + 1,
                            end_gnote,
                            current_offset,
                            end_offset,
                            displacement
                            );
                else {
                    if (current_gnote->get_start() < *current_offset && *current_offset < current_gnote->get_end())
                    {
                        using LR_type = decltype(current_gnote->split_at_offset(*current_offset));
                        LR_type left_right;
                        if constexpr (std::is_same_v<TieTreatment, CreateTies>) {
                            left_right = current_gnote->split_at_offset(*current_offset);
                        }
                        else if constexpr (std::is_same_v<TieTreatment, SeverTies>) {
                            left_right = current_gnote->split_and_sever_ties_at_offset(*current_offset);
                        }
                        split_at_offsets_helper<TieTreatment>(
                                current_gnote + 1,
                                end_gnote,
                                current_offset + 1,
                                end_offset,
                                displacement + 1
                                );
                        *(current_gnote + displacement) = left_right.first;
                        *(current_gnote + displacement + 1) = left_right.second;
                        return;
                    }
                    else
                    {
                        split_at_offsets_helper<TieTreatment>(
                                current_gnote + 1,
                                end_gnote,
                                current_offset,
                                end_offset,
                                displacement
                                );
                        return;
                    }
                }
            }
        }

        // Must NOT!! request to cut at X if there is a gnote with interval [X, _)
        template <typename TieTreatment, template<typename...> class Vec = std::vector>
        void split_at_offsets(Vec<OffsetT> offsets) {
            gnotes.reserve(offsets.size());
            std::fill_n(std::back_inserter(gnotes), offsets.size(), SimpleNoteT());
            split_at_offsets_helper<TieTreatment>(
                    gnotes.begin(),
                    (gnotes.end() - offsets.size() - 1),
                    offsets.begin(),
                    offsets.end(),
                    0);
        }

        generator<const typename GNoteT::SimpleNoteT&>
        simple_note_iter() const
        {
            for (const auto& gnote: this->gnotes) {
                if (std::holds_alternative<typename GNoteT::SimpleNoteT>(gnote)) {
                    co_yield std::get<typename GNoteT::SimpleNoteT>(gnote);
                }
                else if (std::holds_alternative<typename GNoteT::TupletT>(gnote)) {
                    auto& tup = std::get<typename GNoteT::TupletT>(gnote);
                    for (const auto& sn: tup.notes) {
                        co_yield sn;
                    }
                }
            }
        }

        generator<typename GNoteT::SimpleNoteT&> simple_note_itermut()
        {
            for (auto& sn : simple_note_iter()) co_yield const_cast<typename GNoteT::SimpleNoteT&>(sn);
        }

        Part<GNoteT, GNoteCont> transpose_by(m32::PsType displacement)
        {
            Part<GNoteT, GNoteCont> res = *this;
            for (typename GNoteT::SimpleNoteT& sn : res.simple_note_itermut()) {
                for (m32::Pitch& pt: sn.pitches) {
                    pt.transpose(displacement);
                }
            }
            return res;
        }

        template <typename T, T f>
        using F = typename CmpField<Self>::template F<T, f>;

        struct PartNameCmp : F<decltype(&Self::part_name), &Self::part_name> {};
        struct KeySigCmp : F<decltype(&Self::key_sig), &Self::key_sig> {};
        struct ClefSignCmp : F<decltype(&Self::clef_sign), &Self::clef_sign> {};
        struct TimeSigCmp : F<decltype(&Self::time_sig), &Self::time_sig> {};
        template <typename... HasherSet>
        struct HashCmp : F<decltype(&Self::template hash_self<HasherSet...>), &Self::template hash_self<HasherSet...>> {};

        using PartAllowedComps_ = mpl::set<
                PartNameCmp,
                KeySigCmp,
                ClefSignCmp,
                TimeSigCmp
                >;

        using PartAllowedComps = mpl::joint_view<
                PartAllowedComps_,
                typename GNoteT::GNoteAllowedComps
                >;

        template<typename HasherCmpT, typename... Comps>
        bool _eq (const Self& other) const {
            using comparators_s = mpl::set<Comps...>;
            return k_eq<HasherCmpT, comparators_s>(other);
        }

        template <typename HasherCmpT, typename Set>
        bool k_eq(const Self& other) const {
            using comparators_s = Set;
            using comparators_well_defined = typename mpl::fold<
                comparators_s,
                mpl::true_,
                mpl::and_<
                    mpl::placeholders::_1,
                    mpl::contains<PartAllowedComps, mpl::placeholders::_2>
                >
            >::type;
            static_assert(comparators_well_defined::value, "Unknown comparators: Part");

            using part_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::has_key<PartAllowedComps_, mpl::placeholders::_1>
                >::type;
            using gn_comparators_v = typename mpl::filter_view<
                comparators_s,
                mpl::contains<
                    typename GNoteT::GNoteAllowedComps,
                    mpl::placeholders::_1>
                >::type;

            struct part_and_ : And_Comps<Self>::template K<part_comparators_v> {};
            const auto inner_gnote_and_ = std::mem_fn(&GNoteT::template k_eq<gn_comparators_v>);
            return
                part_and_::cmp(*this, other) &&
                HasherCmpT::cmp(*this, other) &&
                std::equal(gnotes.cbegin(), gnotes.cend(), other.gnotes.cbegin(), inner_gnote_and_) ;
        }

        bool exact_eq(const Self& other)
        {
            return _eq<
                HashCmp<>, // must be here for now
                ClefSignCmp,
                KeySigCmp,
                TimeSigCmp,

                typename GNoteT::StartCmp,
                typename GNoteT::EndCmp,
                typename GNoteT::LengthCmp,

                typename GNoteT::SimpleNoteT::PitchCmpExact,
                typename GNoteT::SimpleNoteT::LyricCmp,
                typename GNoteT::SimpleNoteT::TieInfoCmp,
                typename GNoteT::SimpleNoteT::DynamicCmp,
                typename GNoteT::SimpleNoteT::ColorCmp,

                typename GNoteT::TupletT::NormalActualCmp
                >(other);
        }

        friend std::ostream& operator<<(
                std::ostream& os,
                const Part<GNoteT, GNoteCont>& self)
        {
            os << fmt::format("Part<clef={} | key={} | time={}/{} | name={}>",
                              self.clef_sign,
                              self.key_sig,
                              self.time_sig.first, self.time_sig.second,
                              self.part_name)
               << std::endl;

            for (const auto& gnote : self.gnotes)
                os << '\t' << gnote << std::endl;
            return os;
        }
    };

    template <
            typename OffsetT,
            typename DurationT,
            typename GNoteT,
            template < typename, typename... > class MeaCont,
            template < typename, typename... > class GNoteCont>
    class MeasuredPart
            : public NoteHasherMixin<MeasuredPart<OffsetT, DurationT, GNoteT, MeaCont, GNoteCont>, GNoteT>
            , public PartInfoHasherMixin<MeasuredPart<OffsetT, DurationT, GNoteT, MeaCont, GNoteCont>, GNoteT>
            , public HashSelfMixin<MeasuredPart<OffsetT, DurationT, GNoteT, MeaCont, GNoteCont>, GNoteT>
    {
    public:
        using MeasureT = m32::Measure<OffsetT, DurationT, GNoteT, GNoteCont>;
        std::string part_name = "";
        KeySigType key_sig = KeySigType(0);
        ClefSignType clef_sign = ClefSignType('G');
        TimeSigType time_sig = TimeSigType(4, 4);

        MeaCont<MeasureT> measures;
        mutable std::unique_ptr<Botan::HashFunction> sha = nullptr;
        using Self = MeasuredPart<OffsetT, DurationT, GNoteT, MeaCont, GNoteCont>;

        MeasuredPart() = default;
        MeasuredPart(
                std::string part_name,
                KeySigType key_sig,
                ClefSignType clef_sign,
                TimeSigType time_sig
                )
                : part_name {std::move(part_name)}
                , key_sig {key_sig}
                , clef_sign {clef_sign}
                , time_sig {std::move(time_sig)}
                , sha { nullptr }
        {}

        MeasuredPart(const Self& other)
            : part_name(other.part_name)
            , key_sig(other.key_sig)
            , clef_sign(other.clef_sign)
            , time_sig(other.time_sig)
            , measures(other.measures)
            , sha { nullptr } {};

        template <typename Arch>
        void serialize(Arch& arch) {
            arch(
                    CEREAL_NVP(part_name),
                    CEREAL_NVP(key_sig),
                    CEREAL_NVP(clef_sign),
                    CEREAL_NVP(time_sig),
                    CEREAL_NVP(measures)
            );
        }

        void init_sha() const { if (!sha) sha = Botan::HashFunction::create("SHA-512"); }

        explicit MeasuredPart(decltype(measures)&& meas)
            : measures {std::move(meas)}
        {}

        [[nodiscard]] Part<GNoteT, GNoteCont> flatten() const
        {
            Part<GNoteT, GNoteCont> flat(part_name, key_sig, clef_sign, time_sig);
            auto number_of_gnotes = std::transform_reduce(
                    measures.cbegin(), measures.cend(),
                    0,
                    std::plus<>(),
                    [](const MeasureT& measure) { return measure.gnotes.size(); });
            flat.gnotes.reserve(number_of_gnotes);

            auto offset_so_far = OffsetT{0, 1};
            for (const auto& measure: measures) {
                for (const auto& gnote: measure.gnotes) {
                    auto new_gnote = gnote;
                    new_gnote.template set_start<KeepLength>(offset_so_far);
                    offset_so_far += new_gnote.get_length();
                    flat.gnotes.push_back(new_gnote);
                }
            }

            assert(number_of_gnotes == flat.gnotes.capacity());
            return flat;
        }

        [[nodiscard]] generator<const typename GNoteT::SimpleNoteT&>
        simple_note_iter() const
        {
            for (const MeasureT& measure: measures) {
                for (const GNoteT& gnote: measure.gnotes) {
                    if (auto* snote_ptr = std::get_if<typename GNoteT::SimpleNoteT>(&gnote))
                        co_yield *snote_ptr;
                    else if (auto* tuplet_ptr = std::get_if<typename GNoteT::TupletT>((&gnote)))
                        for (auto& simple_note: tuplet_ptr->notes) co_yield simple_note;
                }
            }
        }

        void append_simple_note(const typename GNoteT::SimpleNoteT& simple_note)
        {
            m32::GNote<OffsetT, DurationT> gnote = simple_note;
            append_gnote(gnote);
        }

        void append_gnote(GNoteT _gnote)
        {
            auto measure_length = DurationT{ time_sig.first * 4, time_sig.second };
            if (measures.empty())
                measures.emplace_back(
                        Offset{0, 1},
                        measure_length,
                        0,
                        std::vector<GNoteT>());

            if (!measures.empty() && !measures.back().gnotes.empty()) {
                auto last_gnote = measures.back().gnotes.back();
                _gnote.template displace_start<KeepLength>(last_gnote.get_end());
            }
            boost::container::small_vector<GNoteT, 10> _tgnotes{ _gnote };
            auto cur_note = _tgnotes.begin();
            while (cur_note != _tgnotes.end()) {
                auto& current_measure = measures.back();
                cur_note->template set_start<KeepLength>(
                        current_measure.gnotes.empty()
                        ? current_measure.start
                        : current_measure.start + current_measure.gnotes.back().get_end());

                if (current_measure.template does_swallow(cur_note->const_interval())) {
                    cur_note->template displace_start<KeepLength>(-current_measure.start);
                    current_measure.gnotes.push_back(std::move(*cur_note));
                    std::advance(cur_note, 1);
                }
                else if (current_measure.end == cur_note->get_start()) {
                    measures.template emplace_back(
                            current_measure.end,
                            measure_length,
                            current_measure.measure_number + 1,
                            std::vector<GNoteT>());

                    auto& new_current_measure = measures.back();
                    cur_note->template displace_start<KeepLength>(-new_current_measure.start);
                    new_current_measure.gnotes.push_back(std::move(*cur_note));
                    std::advance(cur_note, 1);
                }
                else if (current_measure.template does_overlap_with(cur_note->const_interval())) {
                    auto [left, right] = cur_note->split_at_offset(current_measure.end);

                    // shift offset
                    left.template displace_start<KeepLength>(-current_measure.start);
                    current_measure.gnotes.push_back(std::move(left));

                    measures.template emplace_back(
                            current_measure.end,
                            measure_length,
                            current_measure.measure_number + 1,
                            std::vector<GNoteT>());
                    auto it_after = _tgnotes.erase(cur_note);
                    cur_note = _tgnotes.insert(it_after, std::move(right));
                }
                else assert(false && "Unreachable");
            }

            assert(std::all_of(
                    measures.cbegin(), std::prev(measures.cend()),
                    [](const auto& mea) { return mea.get_elements_acc_duration() == mea.length; }
                    ));
        }

        template <typename T, T f>
        using F = typename CmpField<Self>::template F<T, f>;

        struct PartNameCmp : F<decltype(&Self::part_name), &Self::part_name> {};
        struct KeySigCmp : F<decltype(&Self::key_sig), &Self::key_sig> {};
        struct ClefSignCmp : F<decltype(&Self::clef_sign), &Self::clef_sign> {};
        struct TimeSigCmp : F<decltype(&Self::time_sig), &Self::time_sig> {};
        template <typename... HasherSet>
        struct HashCmp : F<decltype(&Self::template hash_self<HasherSet...>), &Self::template hash_self<HasherSet...>> {};

        using PartAllowedComps_ = mpl::set<
                PartNameCmp,
                KeySigCmp,
                ClefSignCmp,
                TimeSigCmp
        >;

        using PartAllowedComps =
                mpl::joint_view<
                        mpl::joint_view<
                            PartAllowedComps_,
                            typename GNoteT::GNoteAllowedComps
                        >,
                        typename MeasureT::MeaAllowedComps>;

        template<typename HasherCmpT, typename... Comps>
        bool _eq (const Self& other) const {
            using comparators_s = mpl::set<Comps...>;
            return k_eq<HasherCmpT, comparators_s>(other);
        }

        template <typename HasherCmpT, typename Set>
        bool k_eq(const Self& other) const {
            using comparators_s = Set;
            using comparators_well_defined = typename mpl::fold<
                    comparators_s,
                    mpl::true_,
                    mpl::and_<
                            mpl::placeholders::_1,
                            mpl::contains<PartAllowedComps, mpl::placeholders::_2>
                    >
            >::type;
            static_assert(comparators_well_defined::value, "Unknown comparators: MPart");

            using part_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::has_key<PartAllowedComps_, mpl::placeholders::_1>
            >::type;
            using gn_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::contains<
                            typename GNoteT::GNoteAllowedComps,
                            mpl::placeholders::_1>
            >::type;

            struct mpart_and_ : And_Comps<Self>::template K<part_comparators_v> {};
            const auto inner_measure_and_ = std::mem_fn(&MeasureT::template k_eq<gn_comparators_v>);
            return
                mpart_and_::cmp(*this, other) &&
                HasherCmpT::cmp(*this, other) &&
                measures.size() == other.measures.size() &&
                std::equal(measures.cbegin(), measures.cend(), other.measures.cbegin(), inner_measure_and_) ;
        }

        [[nodiscard]] bool exact_eq(const Self& other) const
        {
            return _eq<
                    ClefSignCmp,
                    KeySigCmp,
                    TimeSigCmp,

                    typename MeasureT::MeaNumberCmp,
                    typename MeasureT::MeaClefSignCmp,
                    typename MeasureT::MeaKeySigCmp,
                    typename MeasureT::MeaTimeSig,

                    typename GNoteT::TupletT::NormalCmp,
                    typename GNoteT::TupletT::ActualCmp,

                    typename GNoteT::SimpleNoteT::PitchCmpExact,
                    typename GNoteT::SimpleNoteT::LyricCmp,
                    typename GNoteT::SimpleNoteT::TieInfoCmp,
                    typename GNoteT::SimpleNoteT::DynamicCmp,
                    typename GNoteT::SimpleNoteT::ColorCmp
            >(other);
        }

        friend std::ostream& operator<<(
                std::ostream& os,
                const MeasuredPart<OffsetT, DurationT, GNoteT, MeaCont, GNoteCont>& self)
        {
            os << fmt::format("MeasuredPart<clef={} | key={} | time={}/{} | name={} >",
                              self.clef_sign,
                              self.key_sig,
                              self.time_sig.first, self.time_sig.second,
                              self.part_name)
                << std::endl;

            for (const auto& currentMeasure : self.measures)
                os << '\t' << currentMeasure << std::endl;
            return os;
        }
    };
}

#endif //AIDA_PART_H
