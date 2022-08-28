//
// Created by dop on 12/26/21.
//

#ifndef AIDA_DURATION_H
#define AIDA_DURATION_H

#include <iostream>
#include <optional>
#include <boost/container/flat_map.hpp>
#include <boost/container/small_vector.hpp>

#include "types.h"

namespace m32 {
    enum class DurationName {
        unspecified,
        maxima,
        longa,
        breve,
        whole,
        half,
        quarter,
        eighth,
        dur16th,
        dur32nd,
        dur64th,
        dur128th,
        dur256th,
        dur512nd,
        dur1024th
    };

    struct Infimum{};
    struct Supremum{};
    struct Open{};
    struct Closed{};

    const char* duration_str_from_name(DurationName);
    DurationName duration_name_from_str(const char*);

    m32::Duration duration_from_duration_name(DurationName);
    DurationName duration_name_from_duration(m32::Duration);

    template<typename Extrema, typename Topology>
    std::optional<m32::Duration> duration_st(m32::Duration duration)
    {
        static const boost::container::flat_map<m32::Duration, DurationName> _duration_to_name =
                {
                        {{32, 1}, DurationName::maxima},
                        {{16, 1}, DurationName::longa},
                        {{8, 1}, DurationName::breve},
                        {{4, 1}, DurationName::whole},
                        {{2, 1}, DurationName::half},
                        {{1, 1}, DurationName::quarter},
                        {{1, 2}, DurationName::eighth},
                        {{1, 4}, DurationName::dur16th},
                        {{1, 8}, DurationName::dur32nd},
                        {{1, 16}, DurationName::dur64th},
                        {{1, 32}, DurationName::dur128th},
                        {{1, 64}, DurationName::dur256th},
                        {{1, 128}, DurationName::dur512nd},
                        {{1, 256}, DurationName::dur1024th},
                };

        const auto LARGEST_LENGTH = std::prev(_duration_to_name.end())->first;
        const auto SMALLEST_LENGTH = _duration_to_name.begin()->first;

        if constexpr(std::is_same_v<Extrema, Infimum>) {
            auto comp = [](const m32::Duration& mock, const auto& pair) {
                if constexpr(std::is_same_v<Topology, Open>) {
                    // open we want first > mock, closed we want first >= mock
                    // upper_bound returns first greater than
                    return mock < pair.first;
                }
                else if constexpr(std::is_same_v<Topology, Closed>) {
                    return mock <= pair.first;
                }
            };

            auto it = std::upper_bound(
                    _duration_to_name.cbegin(), _duration_to_name.cend(),
                    duration,
                    comp
            );

            if (it == _duration_to_name.cend()) {
                // if duration in range, then not found must be due to exclusion by openness
                if (SMALLEST_LENGTH <= duration <= LARGEST_LENGTH) return std::nullopt;
                if (duration < SMALLEST_LENGTH) return SMALLEST_LENGTH;
                else return std::nullopt;
            }
            else return it->first;
        }
        else if constexpr(std::is_same_v<Extrema, Supremum>) {
            auto comp = [](const auto& pair, const m32::Duration& mock) {
                if constexpr(std::is_same_v<Topology, Open>) {
                    // open we want first < mock, closed we want first <= mock
                    // open is >=, closed is >
                    return pair.first >= mock;
                }
                else if constexpr(std::is_same_v<Topology, Closed>) {
                    return pair.first > mock;
                }
            };

            auto it = std::lower_bound(
                    _duration_to_name.crbegin(), _duration_to_name.crend(),
                    duration,
                    comp
            );

            if (it == _duration_to_name.crend()) {
                if (SMALLEST_LENGTH <= duration <= LARGEST_LENGTH) return std::nullopt;
                if (duration < SMALLEST_LENGTH) return std::nullopt;
                else return LARGEST_LENGTH;
            }
            else return it->first;
        }
    }

    template<typename Extrema, typename Topology>
    std::optional<m32::DurationName> duration_name_st(m32::Duration duration)
    {
        auto dur = duration_st<Extrema, Topology>(duration);
        if (!dur.has_value()) return std::nullopt;
        return duration_name_from_duration(dur.value());
    }

    boost::container::small_vector<std::pair<m32::Duration, int>, 10>
    decompose_duration_into_primitives(m32::Duration);

    m32::Duration compute_dotted_length(m32::Duration, int dots);
}
#endif //AIDA_DURATION_H
