//
// Created by dmp on 1/19/22.
//

#ifndef AIDA_TINYNOTE_NUMBER_H
#define AIDA_TINYNOTE_NUMBER_H

#include "m32/utility.h"
#include "tinynote/common.h"
#include "tinynote/DebugControl.h"

#include <type_traits>

#include <tao/pegtl.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/rational.hpp>

namespace tinynote {
    using namespace tao::pegtl;
    struct Integer
            : seq <
                opt< sor< one< '+' >, one< '-' > > >,
                plus< digit >
            > {};

    template <typename I>
    struct IntParseActionIntType {
        template <typename Rule>
        struct IntParseAction : nothing < Rule > {};

        template <>
        struct IntParseAction<Integer> {
            template <typename ActionInput>
            static void apply(ActionInput& in, I& out) {
                if constexpr (std::is_same_v<I, int>) { return std::stoi(in.string()); }
                else if constexpr(std::is_same_v<I, long>) { return std::stol(in.string()); }
                else if constexpr(std::is_same_v<I, long long>){ return std::stoll(in.string()); }
            }
        };
    };

    template <typename IntType>
    IntType parse_integer(std::string in) {
        string_input inp (in, "parse_integer");
        return []<typename ParseInput>(ParseInput& in) {
            IntType out;
            parse<Integer, IntParseActionIntType<IntType>::template IntParseAction >( in, out );
            return out;
        }(inp);
    }

    struct IsNotFraction {
        using rule_t = IsNotFraction;

        template<
                tao::pegtl::apply_mode A,
                tao::pegtl::rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename NumType,
                typename... States >
        static bool match(
                ParseInput& in,
                NumType&,
                States&&...)
        {
            auto* c = in.current();
            while (c != in.end()) {
                if (isdigit(*c) || *c == '.') ++c;
                else break;
            }
            if (*c == '/') return false;
            else return true;
        }
    };

    struct FloatDecimal
            : seq <
                    IsNotFraction, // exclude fractions
                    opt< sor< one< '+' >, one< '-' > > >,
                    opt< Integer >,
                    opt< one<'.'> >,
                    opt< Integer >
            > {};

    struct Fraction
            : seq <
                    opt< sor< one< '+' >, one< '-' > > >,
                    must< Integer >,
                    must< one< '/' > >,
                    must< Integer >
            > {};

    struct FloatOrFraction
            : sor< FloatDecimal, Fraction > {};

    template <typename>
    struct is_boost_rational : public std::false_type {};
    template <typename T>
    struct is_boost_rational<boost::rational<T>> : std::true_type {};

    template <typename NumType>
    struct NumberParseAction {
        template <typename Rule>
        struct parse_action : nothing< Rule > {};

        template <>
        struct parse_action< FloatDecimal >
        {
            template<typename ActionInput>
            static void apply(const ActionInput& in, NumType& out)
            {
                if constexpr (std::is_floating_point_v<NumType>) {
                    out = boost::lexical_cast<NumType>(in.string());
                } else if constexpr (is_boost_rational<NumType>::value) {
                    // must be boost::rational then
                    out = m32::to_rational<typename NumType::int_type>(boost::lexical_cast<double>(in.string()));
                } else { static_assert("uncovered case"); }
            }
        };

        template <>
        struct parse_action< Fraction  >
        {
            template<typename ActionInput>
            static void apply(const ActionInput& in, NumType& out)
            {
                auto frac_comps = split_by(in.string(), "/");
                auto num = std::stoll(frac_comps[0]);
                auto den = std::stoll(frac_comps[1]);

                if constexpr (std::is_floating_point_v<NumType>) {
                    out = static_cast<NumType>(num) / den;
                }
                else if constexpr (is_boost_rational<NumType>::value) {
                    // must be boost::rational then
                    out.template assign(
                            static_cast<typename NumType::int_type>(num),
                            static_cast<typename NumType::int_type>(den));
                }
                else {static_assert("uncovered case");}
            }
        };
    };

    template <typename NumType>
    NumType parse_float_or_fraction(std::string in) {
        string_input inp (in, "parse_float_or_fraction");
        return []<typename ParseInput>(ParseInput& in) {
            NumType out;
            parse<
                FloatOrFraction,
                NumberParseAction<NumType>::template parse_action>( in, out );
            return out;
        }(inp);
    }
}

#endif //AIDA_TINYNOTE_NUMBER_H
