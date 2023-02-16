//
// Created by dop on 12/27/21.
//

#ifndef AIDA_TINYNOTE_COMMON_H
#define AIDA_TINYNOTE_COMMON_H

#include <tao/pegtl.hpp>
#include <boost/algorithm/string.hpp>

namespace tinynote {
    using namespace tao::pegtl;

    struct QuotedString
            : if_must<
                one< '"' >,
                until< one< '"' > >
            > {};

    template <typename Content, char OpenBracket = '[', char ClosedBracket = ']'>
    struct BracketedListOf
            : seq <
                pad< one< OpenBracket >, one<' '>>,
                list_must<
                    Content,
                    pad< one< ',' >, one<' ', '\n'> >
                >,
                pad< one< ClosedBracket >, one<' '>>
            > {};

    inline std::vector<std::string>
    strip_spaces_and_split_by(std::string in, const std::string& sep) {
        boost::erase_all(in, " ");
        std::vector<std::string> split_vec;
        boost::split(split_vec, in, boost::is_any_of(sep));
        return split_vec;
    }

    inline std::vector<std::string>
    split_by(std::string in, const std::string& sep) {
        std::vector<std::string> split_vec;
        boost::split(split_vec, in, boost::is_any_of(sep));
        return split_vec;
    }
}
#endif //AIDA_TINYNOTE_COMMON_H
