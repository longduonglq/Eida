// Project AIDA
// Created by Long Duong on 7/3/22.
// Purpose: 
//

#ifndef AIDA_DEBUGPRINT_H
#define AIDA_DEBUGPRINT_H

#include <iostream>
#include <optional>

#define IN_CASE_OF_NULL(char_ptr, default_str) (char_ptr != nullptr ? char_ptr : default_str)
#define IN_CASE_OF_NULL_XMLCHAR(expr, default) (expr != nullptr)? expr : reinterpret_cast<const unsigned char*>(default)

template <typename T>
std::ostream& operator<<(std::ostream& os, std::optional<T> const& val) {
    if (val.has_value()) { return os << val; }
    else { return os << typeid(T).name() << ": nullopt"; }
}

template <class T>
constexpr
std::string_view
type_name()
{
    using namespace std;
#ifdef __clang__
    string_view p = __PRETTY_FUNCTION__;
    return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
    string_view p = __PRETTY_FUNCTION__;
#  if __cplusplus < 201402
    return string_view(p.data() + 36, p.size() - 36 - 1);
#  else
    return string_view(p.data() + 49, p.find(';', 49) - 49);
#  endif
#elif defined(_MSC_VER)
    string_view p = __FUNCSIG__;
    return string_view(p.data() + 84, p.size() - 84 - 7);
#endif
}


#endif //AIDA_DEBUGPRINT_H
