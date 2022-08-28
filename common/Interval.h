//
// Created by lucad on 12/18/21.
//

#ifndef AIDA_INTERVAL_H
#define AIDA_INTERVAL_H

#include "common.h"

#include <iostream>
#include <fmt/format.h>
#include <pugixml.hpp>
#include <type_traits>
#include <cereal/cereal.hpp>

struct KeepStart {};
struct KeepEnd {};
struct KeepLength {};

template <typename DurationType, typename OffsetType>
concept IsDifferenceTypeOf = std::is_convertible_v<
        decltype(std::declval<OffsetType>() - std::declval<OffsetType>()),
        DurationType>;

template <typename OffsetType, typename DurationType = OffsetType>
requires IsDifferenceTypeOf<DurationType, OffsetType>
class Interval {
public:
    using OffsetT = OffsetType;
    using DurationT = DurationType;

    OffsetType start, end;
    DurationType length;

    static Interval<OffsetType, DurationType> from_end_points(OffsetType start, OffsetType end) {
        return Interval(start, end - start);
    }

    Interval() : start{}, end{}, length{}
    {}

    Interval(OffsetType start, DurationType length) :
        start {start},
        end {static_cast<OffsetType>(start + length)},
        length {length}
    {}

    template <typename Keep>
    void set_start(OffsetType _start) {
        start = _start;
        if constexpr (std::is_same_v<Keep, KeepEnd>) {
            length = end - start;
        }
        else if constexpr (std::is_same_v<Keep, KeepLength>) {
            end = start + length;
        }
        else {
            static_assert(always_false_v<OffsetType>, "Unknown or Wrong Tag <Keep_>");
        }
    }

    template <typename Keep>
    void set_length(DurationType _length) {
        length = _length;
        if constexpr (std::is_same_v<Keep, KeepStart>) {
            end = start + length;
        }
        else if constexpr (std::is_same_v<Keep, KeepEnd>) {
            start = end - length;
        }
        else {
            static_assert(always_false_v<DurationType>, "Unknown or wrong tag <Keep_>");
        }
    }

    template <typename Keep>
    void set_end(OffsetType _end) {
        end = _end;
        if constexpr (std::is_same_v<Keep, KeepStart>) {
            length = end - start;
        }
        else if constexpr (std::is_same_v<Keep, KeepLength>) {
            start = end - length;
        }
        else { static_assert(always_false_v<OffsetType>, "Unknown or wrong tag <Keep_>");}
    }

    template <typename Keep, typename DisplacementType>
    void displace_start(DisplacementType displace)
    {
        set_start<Keep>(start + displace);
    }

    template <typename Keep, typename DeltaType>
    void change_length_by(DeltaType delta) {
        set_length<Keep>(length + delta);
    }

    template <typename Keep, typename ScalarType>
    void scale_length_by(ScalarType factor) {
        set_length<Keep>(length * factor);
    }

    template <typename OffsetT>
    bool does_contain_offset(OffsetT offset) const {
        return start <= offset && offset < end;
    }

    template <typename U>
    bool does_overlap_with(const Interval<U>& other) const {
        return start < other.end && other.start < end;
    }

    template <typename OtherOffsetType>
    bool does_half_closed_interval_contain_offset(OtherOffsetType offset) const {
        return start <= offset && offset < end;
    }

    template <typename U>
    bool is_disjoint_with(const Interval<U>& other) const {
        return !does_overlap_with(other);
    }

    template <typename U>
    bool does_swallow(const Interval<U>& other) const {
        return start <= other.start && other.end <= end;
    }

    template <typename U>
    bool is_swallowed_by(const Interval<U>& other) const {
        return other.start <= start && end <= other.end;
    }

    bool is_strictly_swallowed_by(const Interval<OffsetType>& other) const {
        return other.start < start && end < other.end;
    }

    template <typename U>
    auto get_intersection_with(const Interval<U>& other) const {
        auto high_bound = std::min(end, other.end);
        auto low_bound = std::max(start, other.start);

        assert( low_bound < high_bound && "Intersection makes no sense");
        return Interval<OffsetType>::from_end_points(low_bound, high_bound);
    }

    template <typename U>
    void merge_with(const Interval<U>& other) {
        assert(start + length == other.start && "Can't merge if not adjacent");
        set_length<KeepStart>(length + other.length);
    }

    template <typename U>
    bool operator== (const Interval<U>& rhs) {
        return ((start == rhs.start) && (end == rhs.end) && (length == rhs.length));
    }

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(start), CEREAL_NVP(end), CEREAL_NVP(length));
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Interval<T>& itv) {
    return os << fmt::format("Interval[{}, {})", itv.start, itv.end);
}

#endif //AIDA_INTERVAL_H
