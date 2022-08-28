// Project AIDA
// Created by Long Duong on 7/3/22.
// Purpose: 
//
#include <fmt/format.h>
#include <iostream>
#include <optional>
#include "Lyric.h"

template <>
struct fmt::formatter<m32::Lyric> : fmt::formatter<std::string> {
    auto format(const m32::Lyric& lyric, fmt::format_context& ctx) {
        return fmt::format_to(ctx.out(), "Lyric[number={}, lyric={}]", lyric.number, lyric.text);
    }
};


