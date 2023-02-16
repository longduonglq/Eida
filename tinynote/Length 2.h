//
// Created by dmp on 1/22/22.
//

#ifndef AIDA_TINYNOTE_LENGTH_H
#define AIDA_TINYNOTE_LENGTH_H

#include <tao/pegtl.hpp>

#include "tinynote/Number.h"
#include "tinynote/common.h"
#include "m32/utility.h"

namespace tinynote {
    using namespace tao::pegtl;

    struct Length : tinynote::FloatOrFraction {};

    template <typename Rule>
    struct LengthParseAction : nothing< Rule > {};

    template <>
    struct LengthParseAction< FloatDecimal >
    {
        template<typename ActionInput, typename I>
        static void apply(const ActionInput &in, boost::rational<I>& length)
        {
            auto length_str = in.string();
            auto length_db = std::stold(length_str);
            length = m32::to_rational<I, decltype(length_db)>(length_db);
        }
    };

    template <>
    struct LengthParseAction< Fraction >
    {
        template<typename ActionInput, typename I>
        static void apply(const ActionInput &in, boost::rational<I>& length)
        {
            auto frac_str = strip_spaces_and_split_by(in.string(), "/");
            auto num = std::stoll(frac_str[0]);
            auto den = std::stoll(frac_str[1]);
            length.template assign(num, den);
        }
    };

    template<typename I>
    requires std::is_integral_v<I>
    boost::rational<I> parse_length(std::string in) {
        string_input inp (in, "parse_length");
        return []<typename ParseInput>(ParseInput& in) {
            boost::rational<I> out;
            parse< Length, LengthParseAction >( in, out );
            return out;
        }(inp);
    }
}

#endif // AIDA_TINYNOTE_LENGTH_H
