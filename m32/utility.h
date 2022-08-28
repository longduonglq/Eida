//
// Created by dop on 12/26/21.
//

#ifndef AIDA_UTILITY_H
#define AIDA_UTILITY_H

#include <boost/rational.hpp>
#include <type_traits>
#include <numeric>

namespace m32 {
    template <typename FloatType, typename IntType>
    requires std::is_integral_v<IntType>
    FloatType rational_to(IntType i) {
        return static_cast<FloatType>(i);
    }

    template <typename FloatType, typename I>
    FloatType rational_to(boost::rational<I> rat) {
        return static_cast<FloatType>(rat.numerator()) / rat.denominator();
    }

    template <typename I, typename FloatType>
    boost::rational<I> to_rational(FloatType f) {
        static_assert(sizeof(double) >= sizeof(decltype(f)));
        auto df = static_cast<double>(f);
        I big_denom = 1000'000;
        df *= big_denom;
        return boost::rational<I>(static_cast<I>(df), big_denom);
    }

    template <typename I>
    boost::rational<I> frac_gcd(boost::rational<I> a, boost::rational<I> b) {
        return boost::rational<I>(std::gcd(a.numerator(), b.numerator()), std::lcm(a.denominator(), b.denominator()));
    }
}
#endif //AIDA_UTILITY_H
