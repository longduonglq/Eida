// Project AIDA
// Created by Long Duong on 12/21/21.
// Purpose: 
//

#include <boost/range/algorithm.hpp>
#include <fmt/format.h>
#include "Pitch.h"

namespace m32 {
    Pitch::Pitch(DiatonicStep step, std::optional<m32::Octave> octave, Alter alter)
            : step{step}
            , octave{octave}
            , alter{alter}
            , ps(step + static_cast<decltype(step)>(alter) + (octave.value_or(4) + 1) * 12)
            {}

    void Pitch::transpose(const m32::PsType &halfSteps) { update_ps(ps + halfSteps); }

    bool Pitch::exact_eq(const Pitch &rhs) const {
        return alter == rhs.alter && octave == rhs.octave && step == rhs.step;
    }

    bool Pitch::ps_eq(const Pitch& rhs) const { return ps == rhs.ps; }

    void Pitch::update_ps(m32::PsType _ps) {
        auto pc = _ps % 12;
        // Octave octShift = 0;
        if (std::binary_search(DIATONIC_PC.cbegin(), DIATONIC_PC.cend(), pc)) {
            step = DiatonicStep{static_cast<DiatonicStep>( pc )};
        } else if (pc == 1 || pc == 6 || pc == 8) {
            alter = Alter::Sharp;
            step = DiatonicStep(pc - 1);
        } else if (pc == 10 || pc == 3) {
            alter = Alter::Flat;
            step = DiatonicStep(pc + 1);
        } else { assert (false && "Unreachable"); }

        if (octave.has_value()) { octave.emplace(_ps / 12.0 - 1); }
        this->ps = _ps;
    }

    Pitch::DiatonicStep Pitch::diatonic_step_from_char(char step) {
        auto normalStep = toupper(step);
        switch (normalStep) {
            case 'A': return DiatonicStep::A;
            case 'B': return DiatonicStep::B;
            case 'C': return DiatonicStep::C;
            case 'D': return DiatonicStep::D;
            case 'E': return DiatonicStep::E;
            case 'F': return DiatonicStep::F;
            case 'G': return DiatonicStep::G;
            default: assert(false && "Unknown char to diatonic step");
        }
    }

    const char *Pitch::string_from_diatonic_step(DiatonicStep step) {
        switch (step) {
            case Pitch::DiatonicStep::A: return "A";
            case Pitch::DiatonicStep::B: return "B";
            case Pitch::DiatonicStep::C: return "C";
            case Pitch::DiatonicStep::D: return "D";
            case Pitch::DiatonicStep::E: return "E";
            case Pitch::DiatonicStep::F: return "F";
            case Pitch::DiatonicStep::G: return "G";
            default: assert(false && "unknown diatonic step");
        }
    }

    std::ostream& operator<<(std::ostream& os, const Pitch::DiatonicStep& step) {
        return os << Pitch::string_from_diatonic_step(step);
    }

    std::ostream &operator<<(std::ostream &os, const Pitch &m) {
        std::string acc;
        if (m.alter != 0)
            acc = m.alter == Pitch::Alter::Sharp ? "#" : "b";

        std::string otv;
        if (m.octave.has_value())
            otv = std::to_string(m.octave.value());

        char step;
        switch (m.step) {
            case Pitch::DiatonicStep::A: step = 'A'; break;
            case Pitch::DiatonicStep::B: step = 'B'; break;
            case Pitch::DiatonicStep::C: step = 'C'; break;
            case Pitch::DiatonicStep::D: step = 'D'; break;
            case Pitch::DiatonicStep::E: step = 'E'; break;
            case Pitch::DiatonicStep::F: step = 'F'; break;
            case Pitch::DiatonicStep::G: step = 'G'; break;
        }

        return os << fmt::format("{}{}{}", step, acc, otv);
    }

}