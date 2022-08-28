//
// Created by dop on 12/26/21.
//

#include <unordered_map>
#include <map>
#include <cmath>
#include <stdexcept>
#include <fmt/format.h>

#include "Duration.h"
#include "M32Except.h"

namespace m32 {
    class DurationExcept : public M32Exception {
    public:
        using M32Exception::M32Exception;
    };

    const char* duration_str_from_name(DurationName name) {
        switch (name) {
            case DurationName::maxima: return "maxima";
            case DurationName::longa: return "longa";
            case DurationName::breve: return "breve";
            case DurationName::whole: return "whole";
            case DurationName::half: return "half";
            case DurationName::quarter: return "quarter";
            case DurationName::eighth: return "eighth";
            case DurationName::dur16th: return "16th";
            case DurationName::dur32nd: return "32nd";
            case DurationName::dur64th: return "64th";
            case DurationName::dur128th: return "128th";
            case DurationName::dur256th: return "256th";
            case DurationName::dur512nd: return "512nd";
            case DurationName::dur1024th: return "1024th";
            default: return "unspecified";
        }
    }

    DurationName duration_name_from_str(const char* duration_str) {
        static const std::unordered_map<std::string, DurationName> _str_to_name =
                {
                        {"maxima", DurationName::maxima},
                        {"longa", DurationName::longa},
                        {"breve", DurationName::breve},
                        {"whole", DurationName::whole},
                        {"half", DurationName::half},
                        {"quarter", DurationName::quarter},
                        {"eighth", DurationName::eighth},
                        {"16th", DurationName::dur16th},
                        {"32nd", DurationName::dur32nd},
                        {"64th", DurationName::dur64th},
                        {"128th", DurationName::dur128th},
                        {"256th", DurationName::dur256th},
                        {"512nd", DurationName::dur512nd},
                        {"1024th", DurationName::dur1024th},
                };

        auto durationNameSearch = _str_to_name.find(duration_str);
        if (durationNameSearch != _str_to_name.end()) return durationNameSearch->second;
        else throw DurationExcept("No matching duration found for {}", duration_str);
    }

    m32::Duration duration_from_duration_name(DurationName duration_name) {
        static const std::unordered_map<DurationName, m32::Duration> _name_to_duration =
                {
                        {DurationName::maxima, {32, 1}},
                        {DurationName::longa, {16, 1}},
                        {DurationName::breve, {8, 1}},
                        {DurationName::whole, {4, 1}},
                        {DurationName::half, {2, 1}},
                        {DurationName::quarter, {1, 1}},
                        {DurationName::eighth, {1, 2}},
                        {DurationName::dur16th, {1, 4}},
                        {DurationName::dur32nd, {1, 8}},
                        {DurationName::dur64th, {1, 16}},
                        {DurationName::dur128th, {1, 32}},
                        {DurationName::dur256th, {1, 64}},
                        {DurationName::dur512nd, {1, 128}},
                        {DurationName::dur1024th, {1, 256}},
                };

        auto durationSearch = _name_to_duration.find(duration_name);
        if (durationSearch != _name_to_duration.end()) return durationSearch->second;
        else throw DurationExcept("No matching duration found for {}",
                                              duration_str_from_name(duration_name));
    }

    DurationName duration_name_from_duration(m32::Duration numeric_duration) {
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

        auto lb = _duration_to_name.find(numeric_duration);
        if (lb == _duration_to_name.end())
            throw DurationExcept("No matching exception found for {}", numeric_duration);
        else return lb->second;
    }

    boost::container::small_vector<std::pair<m32::Duration, int>, 10>
    decompose_duration_into_primitives(m32::Duration duration)
    {
        auto decomposition = boost::container::small_vector<std::pair<m32::Duration, int>, 10>();

        m32::Duration remain = duration;

        m32::Duration maximal_primitive_duration;
        while (remain >= maximal_primitive_duration || remain > 0)
        {
            maximal_primitive_duration = duration_st<Supremum, Closed>(remain).value();

            remain -= maximal_primitive_duration;
            if (!decomposition.empty())
            {
                auto& [last_duration, last_dots] = decomposition.back();
                // check if can be included as dot to the last component
                if (maximal_primitive_duration ==
                    last_duration / static_cast<int>(std::pow(2, last_dots + 1)))
                {
                    last_dots++;
                }
                else decomposition.emplace_back(maximal_primitive_duration, 0);
            }
            else decomposition.emplace_back(maximal_primitive_duration, 0);
        }
        if (remain != 0)
            throw DurationExcept("Duration {} not decomposable", duration);
        return decomposition;
    }

    m32::Duration compute_dotted_length(m32::Duration duration, int dots) {
        assert(dots >= 0);
        m32::Duration res {0, 1};
        while (dots >= 0) {
            res += (duration / static_cast<m32::Duration::int_type>(std::pow(2, dots)));
            dots --;
        }
        return res;
    }
}