//
// Created by dop on 12/26/21.
//

#ifndef AIDA_M32EXCEPT_H
#define AIDA_M32EXCEPT_H

#include <exception>
#include <string>
#include <fmt/format.h>

namespace m32 {
    class M32Exception : public std::exception {
    public:
        template <typename ... Args>
        explicit M32Exception(std::string_view fmt, Args... args)
                : msg (fmt::vformat(fmt, fmt::make_format_args(args...)))
        {}
        virtual const char* what() const noexcept {return msg.c_str();}
    private:
        std::string msg;
    };
}
#endif //AIDA_M32EXCEPT_H
