//
// Created by dop on 12/27/21.
//

#ifndef AIDA_PITCH_PARSER_H
#define AIDA_PITCH_PARSER_H

#include <tao/pegtl.hpp>

#include "m32/Pitch.h"
#include "m32/types.h"

#include "ParseException.h"

/*
 * Pitch: <Step><(#/b)?><octave?>
 * */

namespace tinynote {
    using namespace tao::pegtl;
    namespace PitchInternal {
        struct Step
                : ranges<'a', 'g', 'A', 'G'> {};

        struct Alter
                : sor<one<'#'>, one<'b'> > {};

        struct Octave
                : plus< digit > {};

        struct Pitch
                : seq<
                        Step,
                        opt<Alter>,
                        opt<Octave>
                > {};

        template<typename Rule>
        struct PitchParseAction : nothing<Rule> {};

        template<>
        struct PitchParseAction<Step> {
            template<typename ActionInput>
            static void apply(const ActionInput &in, m32::Pitch &pitch) {
                auto step_str = in.string();
                auto &step = pitch.step;
                if (step_str == "A" || step_str == "a")
                    step = m32::Pitch::DiatonicStep::A;
                else if (step_str == "B" || step_str == "b")
                    step = m32::Pitch::DiatonicStep::B;
                else if (step_str == "C" || step_str == "c")
                    step = m32::Pitch::DiatonicStep::C;
                else if (step_str == "D" || step_str == "d")
                    step = m32::Pitch::DiatonicStep::D;
                else if (step_str == "E" || step_str == "e")
                    step = m32::Pitch::DiatonicStep::E;
                else if (step_str == "F" || step_str == "f")
                    step = m32::Pitch::DiatonicStep::F;
                else if (step_str == "G" || step_str == "g")
                    step = m32::Pitch::DiatonicStep::G;
                else throw ParseException("Unknown step: {}", step_str);
            }
        };

        template<>
        struct PitchParseAction<Alter> {
            template<typename ActionInput>
            static void apply(const ActionInput &in, m32::Pitch &pitch) {
                auto alter_str = in.string();
                auto &alter = pitch.alter;
                if (alter_str == "#") alter = m32::Pitch::Alter::Sharp;
                else if (alter_str == "b") alter = m32::Pitch::Alter::Flat;
            }
        };

        template<>
        struct PitchParseAction<Octave> {
            template<typename ActionInput>
            static void apply(const ActionInput &in, m32::Pitch &pitch) {
                auto oct_str = in.string();
                auto &octave = pitch.octave;
                octave = std::stoi(oct_str);
            }
        };
    }

    inline m32::Pitch
    parse_pitch(std::string in) {
        string_input inp (in, "parse_pitch");
        return []<typename ParseInput>(ParseInput& in) {
            m32::Pitch pitch(m32::Pitch::DiatonicStep::A, std::nullopt, m32::Pitch::Alter::No);
            parse<PitchInternal::Pitch, PitchInternal::PitchParseAction> (in, pitch);
            auto pitch_ = m32::Pitch(pitch.step, pitch.octave, pitch.alter);
            return pitch_ ;
        }(inp);
    }

}
#endif //AIDA_PITCH_PARSER_H
