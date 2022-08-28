//
// Created by dop on 12/27/21.
//

#include <catch2/catch.hpp>
#include <iostream>
#include <tao/pegtl.hpp>

#include "tinynote/SimpleNoteParser.h"

using namespace tao::pegtl;

template <typename Rule>
struct Action : nothing<Rule> {};

template <>
struct Action <tinynote::QuotedString>
{
    template<typename ActionInput>
    static void apply(const ActionInput &in, std::vector<std::string>& out) { out.push_back(in.string()); }
};

TEST_CASE("QuotedString") {
    string_input in ("\"I don t like this\"", "sdf");
    std::vector<std::string> out;
    parse<tinynote::QuotedString, Action> (in, out);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0] == "\"I don t like this\"");
}

TEST_CASE("BracketedListOf") {
    string_input in ("[\"asd\", \"grthr\"]", "sdf");
    std::vector<std::string> out;
    parse<tinynote::BracketedListOf< tinynote::QuotedString >, Action>  (in, out);
    REQUIRE(out.size() == 2);
    REQUIRE(out[0] == "\"asd\"");
    REQUIRE(out[1] == "\"grthr\"");
}