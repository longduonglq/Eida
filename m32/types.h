// Project AIDA
// Created by Long Duong on 12/22/21.
// Purpose: 
//

#ifndef AIDA_TYPES_H
#define AIDA_TYPES_H

#include <cstdint>
#include <boost/rational.hpp>

#include <cereal/cereal.hpp>

namespace m32 {
    using PitchClass = std::int8_t;
    using Octave = std::int8_t;
    using PsType = std::int32_t;

    using KeySigType = int8_t;
    using DivisionType = int32_t;
    using TimeSigComponentType = uint8_t;
    using TimeSigType = std::pair<TimeSigComponentType, TimeSigComponentType>;
    using ClefSignType = char;

    using Offset = boost::rational<DivisionType>;
    using Duration = boost::rational<DivisionType>;
}

namespace cereal {
    template <typename Arch>
    void serialize(Arch& arch, m32::Offset& offset) {
        m32::DivisionType num = offset.numerator(), denom = offset.denominator();
        arch(
                cereal::make_nvp("numerator", num),
                cereal::make_nvp("denominator", denom)
        );
    }
}

#endif //AIDA_TYPES_H
