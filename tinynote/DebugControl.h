// Project AIDA
// Created by Long Duong on 5/31/22.
// Purpose: 
//

#ifndef AIDA_DEBUGCONTROL_H
#define AIDA_DEBUGCONTROL_H

#include <fmt/format.h>
#include <iostream>

#include <tao/pegtl.hpp>
#include <tao/pegtl/internal/enable_control.hpp>

namespace tinynote {
    using namespace tao::pegtl;

    template< typename Rule >
    struct DebugControl
    {
        static constexpr bool enable = tao::pegtl::internal::enable_control< Rule >;

        template< typename ParseInput, typename... States >
        static void start( const ParseInput& in, States&&... /*unused*/ ) noexcept
        {
            fmt::print("rule: {} \n\t", std::string(demangle<Rule>()));
            fmt::print("++ start: {}\n\n", std::string_view(in.current(), in.size()));
        }

        template< typename ParseInput, typename... States >
        static void success( const ParseInput& in, States&&... /*unused*/ ) noexcept
        {
            fmt::print("rule: {} \n\t", std::string(demangle<Rule>()));
            fmt::print("++ success: {}\n\n", std::string_view(in.current(), in.size()));
        }

        template< typename ParseInput, typename... States >
        static void failure( const ParseInput& in, States&&... /*unused*/ ) noexcept
        {
            // fmt::print("rule: {} \n\t", std::string(demangle<Rule>()));
            // fmt::print("++ failure: {}\n", std::string_view(in.current(), in.size()));
        }

        template< typename ParseInput, typename... States >
        [[noreturn]] static void raise( const ParseInput& in, States&&... /*unused*/ )
        {
#if defined( __cpp_exceptions )
            throw tao::pegtl::parse_error( "parse error matching " + std::string( demangle< Rule >() ), in );
#else
            static_assert( internal::dependent_false< Rule >, "exception support required for normal< Rule >::raise()" );
         (void)in;
         std::terminate();
#endif
        }

        template< template< typename... > class Action,
                typename Iterator,
                typename ParseInput,
                typename... States >
        static auto apply( const Iterator& begin, const ParseInput& in, States&&... st ) noexcept( noexcept( Action< Rule >::apply( std::declval< const typename ParseInput::action_t& >(), st... ) ) )
        -> decltype( Action< Rule >::apply( std::declval< const typename ParseInput::action_t& >(), st... ) )
        {
            fmt::print("rule: {} \n", std::string(demangle<Rule>()));
            fmt::print("++ apply: {}\n", std::string_view(in.current(), in.size()));
            fmt::print("++ action: {}\n\n", std::string (demangle<Action<Rule>>()) );
            const typename ParseInput::action_t action_input( begin, in );
            return Action< Rule >::apply( action_input, st... );
        }

        template< template< typename... > class Action,
                typename ParseInput,
                typename... States >
        static auto apply0( const ParseInput& /*unused*/, States&&... st ) noexcept( noexcept( Action< Rule >::apply0( st... ) ) )
        -> decltype( Action< Rule >::apply0( st... ) )
        {
            return Action< Rule >::apply0( st... );
        }

        template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
        [[nodiscard]] static bool match( ParseInput& in, States&&... st )
        {
            fmt::print("rule: {} \n", std::string(demangle<Rule>()));
            fmt::print("++ match: {}\n", std::string_view(in.current(), in.size()));
            fmt::print("++ action: {}\n\n", std::string (demangle<Action<Rule>>()) );
            if constexpr( tao::pegtl::internal::has_match< bool, Rule, A, M, Action, Control, ParseInput, States... > ) {
                return Action< Rule >::template match< Rule, A, M, Action, Control >( in, st... );
            }
            else {
                return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... );
            }
        }
    };
}

#endif //AIDA_DEBUGCONTROL_H
