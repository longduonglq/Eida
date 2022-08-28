//
// Created by dop on 12/25/21.
//

#ifndef AIDA_TIEINFO_H
#define AIDA_TIEINFO_H

#include <cstdint>

namespace m32 {
    enum TieInfo : int {
        TieNeither = 0,
        TieStart = 1 << 0,
        TieEnd = 1 << 1
    };

    inline constexpr TieInfo operator|(TieInfo a, TieInfo b) {
        return static_cast<TieInfo>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline constexpr TieInfo &operator|=(TieInfo &a, TieInfo b) {
        a = a | b;
        return a;
    }

    inline constexpr TieInfo &operator^=(TieInfo &a, TieInfo b) {
        a = TieInfo(static_cast<int>(a) ^ static_cast<int>(b));
        return a;
    }

    inline constexpr TieInfo &operator&=(TieInfo &a, TieInfo b) {
        a = TieInfo(static_cast<int>(a) & static_cast<int>(b));
        return a;
    }

    inline constexpr TieInfo operator~(TieInfo a) { return TieInfo(~static_cast<int>(a)); }
}

#endif //AIDA_TIEINFO_H
