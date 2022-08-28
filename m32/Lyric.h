//
// Created by dop on 12/25/21.
//

#ifndef AIDA_LYRIC_H
#define AIDA_LYRIC_H

#include <cstdint>
#include <string>
#include <cereal/cereal.hpp>

#include "cmps.h"

namespace m32 {
    struct Lyric {
    public:
        uint8_t number;
        std::string text;

        Lyric() =default;
        Lyric(uint8_t number, std::string text)
            : number {number}
            , text {std::move(text)}
        {}

        template <typename T, T f>
        using F = typename CmpField<Lyric>::template F<T, f>;
        struct NumberCmp : F<decltype(&Lyric::number), &Lyric::number> {};
        struct TextCmp : F<decltype(&Lyric::text), &Lyric::text> {};
        using LyricAllowedComps = mpl::set<
                NumberCmp,
                TextCmp
                >;

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(CEREAL_NVP(number), CEREAL_NVP(text));
        }
    };
}

#endif //AIDA_LYRIC_H
