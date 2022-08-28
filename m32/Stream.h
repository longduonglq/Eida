// Project AIDA
// Created by Long Duong on 8/17/22.
// Purpose: 
//

#ifndef AIDA_STREAM_H
#define AIDA_STREAM_H

#include <vector>
#include <string>
#include <list>
#include <cassert>

#include <cereal/cereal.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/mpl/sort.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/less.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/mpl/has_key.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/filter_view.hpp>

#include <common/generator.hpp>

#include "Pitch.h"
#include "TieInfo.h"
#include "types.h"
#include "cmps.h"

namespace m32 {
    class Spanner {
    public:
        
    };

    template <
            typename GNoteT_,
            template < typename, typename... > class GNoteCont = std::vector>
    class Stream {
    public:
        using GNoteT = GNoteT_;
        GNoteCont< GNoteT > gnotes;
        using SimpleNoteT = typename GNoteT::SimpleNoteT;
        using TupletT = typename GNoteT::TupletT;
        using OffsetT = typename GNoteT::OffsetT;

        Stream() = default;
        template<typename Arch>
        void serialize(Arch& arch) {
            arch(CEREAL_NVP(gnotes));
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

        Stream<GNoteT, GNoteCont> fuse_tied_notes() {
            Stream<GNoteT, GNoteCont> new_stream(*this);
            StableCont<GNoteT> _gnotes(new_stream.gnotes.begin(), new_stream.gnotes.end());

            fuse_tail_(_gnotes, _gnotes.begin());

            new_stream.gnotes.clear();
            new_stream.gnotes.insert(
                    new_stream.gnotes.end(),
                    std::make_move_iterator(_gnotes.begin()),
                    std::make_move_iterator(_gnotes.end()));
            return new_stream;
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

        Stream<GNoteT, GNoteCont> transpose_by(m32::PsType displacement)
        {
            Stream<GNoteT, GNoteCont> res = *this;
            for (typename GNoteT::SimpleNoteT& sn : res.simple_note_itermut()) {
                for (m32::Pitch& pt: sn.pitches) {
                    pt.transpose(displacement);
                }
            }
            return res;
        }

        using Self = Stream<GNoteT, GNoteCont>;
        template <typename T, T f>
        using F = typename CmpField<Self>::template F<T, f>;

        using StreamAllowedComps = typename GNoteT::GNoteAllowedComps;

        template<typename HasherCmpT, typename... Comps>
        bool _eq (const Self& other) const {
            using comparators_s = mpl::set<Comps...>;
            return k_eq<HasherCmpT, comparators_s>(other);
        }

        template <typename Set>
        bool k_eq(const Self& other) const {
            using comparators_s = Set;
            using comparators_well_defined = typename mpl::fold<
                    comparators_s,
                    mpl::true_,
                    mpl::and_<
                    mpl::placeholders::_1,
                    mpl::contains<StreamAllowedComps, mpl::placeholders::_2>
            >
            >::type;
            static_assert(comparators_well_defined::value, "Unknown comparators: Part");

            using gn_comparators_v = typename mpl::filter_view<
                    comparators_s,
                    mpl::contains<
                    typename GNoteT::GNoteAllowedComps,
                    mpl::placeholders::_1>
            >::type;

            const auto inner_gnote_and_ = std::mem_fn(&GNoteT::template k_eq<gn_comparators_v>);
            return std::equal(gnotes.cbegin(), gnotes.cend(), other.gnotes.cbegin(), inner_gnote_and_);
        }

        bool exact_eq(const Self& other)
        {
            return _eq<
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
                const Stream<GNoteT, GNoteCont>& self)
        {
            os << fmt::format("Stream<>")
               << std::endl;

            for (const auto& gnote : self.gnotes)
                os << '\t' << gnote << std::endl;
            return os;
        }
    };
}

#endif //AIDA_STREAM_H
