//
// Created by dop on 12/27/21.
//

#ifndef AIDA_PARSEEXCEPTION_H
#define AIDA_PARSEEXCEPTION_H

#include <stdexcept>
#include <fmt/format.h>

namespace tinynote {
    class ParseException : public std::exception {
    public:
        template<typename ... Args>
        explicit ParseException(std::string_view fmt, Args... args)
                : msg(fmt::vformat(fmt, fmt::make_format_args(args...))) {}

        virtual const char *what() const noexcept { return msg.c_str(); }

    private:
        std::string msg;
    };
}

#endif //AIDA_PARSEEXCEPTION_H
