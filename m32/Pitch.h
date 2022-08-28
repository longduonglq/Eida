// Project AIDA
// Created by Long Duong on 12/21/21.
// Purpose: 
//

#ifndef AIDA_PITCH_H
#define AIDA_PITCH_H

#include <array>
#include <optional>
#include "types.h"

#include <cereal/cereal.hpp>
#include <cereal/types/optional.hpp>

namespace m32 {
    class Pitch {
    public:
        constexpr static std::array<m32::PitchClass, 7> DIATONIC_PC = {0, 2, 4, 5, 7, 9, 11};

        enum DiatonicStep : m32::PitchClass {
            A = 9,
            B = 11,
            C = 0,
            D = 2,
            E = 4,
            F = 5,
            G = 7
        };

        static DiatonicStep diatonic_step_from_char(char step);

        static const char *string_from_diatonic_step(DiatonicStep step);

        enum Alter : int8_t {
            Flat = -1,
            No = 0,
            Sharp = 1
        };

        DiatonicStep step;
        std::optional<m32::Octave> octave;
        Alter alter;
        m32::PsType ps;

        Pitch() =default;
        Pitch(
                DiatonicStep step,
                std::optional<m32::Octave> octave,
                Alter alter
        );

        void transpose(const m32::PsType &halfSteps);

        bool exact_eq(const Pitch &rhs) const;
        bool ps_eq(const Pitch& rhs) const;
        friend std::ostream& operator<<(std::ostream& os, const Pitch::DiatonicStep& step);
        friend std::ostream &operator<<(std::ostream &, const Pitch &);

        template<class Archive>
        void serialize(Archive& archive) {
            archive(
                    CEREAL_NVP(step),
                    CEREAL_NVP(octave),
                    CEREAL_NVP(alter),
                    CEREAL_NVP(ps));
        }

    private:
        void update_ps(m32::PsType _ps);
    };
}

#endif //AIDA_PITCH_H
