// Project AIDA
// Created by Long Duong on 7/2/22.
// Purpose: 
//

#ifndef AIDA_CMPS_H
#define AIDA_CMPS_H

#include <type_traits>
#include <utility>
#include <functional>
#include <boost/fusion/algorithm.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/greater.hpp>
#include <fmt/format.h>
#include <iostream>
#include <botan/botan.h>

#include "types.h"
#include "debugprint.h"

namespace m32 {
    using namespace boost;
    template <typename F>
    struct CmpFromLambda : public F {
        constexpr static auto cmp = F::operator();
    };

    template <typename T>
    concept Coutable = requires(T t) {
        { std::cout << T{} } -> std::convertible_to<std::ostream&>;
    };

    template <typename Self>
    struct CmpField {
        template<
                typename FieldT, FieldT field,
                typename T = std::remove_cvref_t<std::invoke_result_t<FieldT, Self& >>,
                typename Cmp = std::equal_to<T>>
        struct F {
            constexpr static auto cmp = [](const auto &n1, const auto &n2) {
#ifdef SHOW_CMP_MISMATCH
                bool b = Cmp()(std::invoke(field, n1), std::invoke(field, n2));
                if constexpr(Coutable<decltype(std::invoke(field, n1))>) {
                    if (!b) {
                        std::cout << "Comparison of "
                                  << type_name<decltype(field)>()
                                  << " and "
                                  << type_name<decltype(field)>()
                                  << " (ie: "
                                  << std::invoke(field, n1)
                                << " and "
                                << std::invoke(field, n2)
                                << " do not match" << std::endl;}
                }
#endif
                return Cmp()(std::invoke(field, n1), std::invoke(field, n2));
            };
        };
    };

    template <typename Self>
    struct And_Comps {
        template <typename Set>
        struct K {
            constexpr static bool cmp (const Self& n1, const Self& n2) {
                using comparators = Set;
                auto and_ = [&]<typename T>(bool s, T) -> bool { return s && T::cmp(n1, n2); };
                return fusion::fold(comparators{}, true, and_);
            };
        };

        template <typename... Comps>
        struct F : public K<mpl::set<Comps...>> {};
    };

}

#endif //AIDA_CMPS_H
