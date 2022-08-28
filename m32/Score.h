// Project AIDA
// Created by Long Duong on 7/4/22.
// Purpose: 
//

#ifndef AIDA_SCORE_H
#define AIDA_SCORE_H

#include "Part.h"
#include "Tuplet.h"
#include "cmps.h"
#include <string>
#include <memory>
#include <botan/botan.h>
#include <fmt/format.h>
#include <algorithm>
#include <utility>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace m32 {
    template <
            typename OffsetT,
            typename DurationT,
            template < typename, typename... > class MeaCont,
            template < typename, typename... > class GNoteCont,
            template < typename, typename... > class MPartCont>
    class MeasuredScore;

    template <
            typename GNoteT,
            template <typename, typename ...> class GNoteCont = std::vector,
            template <typename, typename...> class PartCont = std::vector
            >
    class Score {
    public:
        using PartT = m32::Part<GNoteT, GNoteCont>;
        std::string work_title;
        PartCont<PartT> parts;
        mutable std::unique_ptr<Botan::HashFunction> sha;
        using Self = Score<PartT, GNoteCont, PartCont>;

        Score(const char* work_title)
            : work_title(work_title)
            , sha{ nullptr }
            {};
        Score(const Self& other)
            : work_title {other.work_title}
            , parts {other.parts}
            , sha { nullptr }
        {}

        template <typename Arch>
        void serialize(Arch& arch) {
            arch(
                    CEREAL_NVP(work_title),
                    CEREAL_NVP(parts)
            );
        }

        void init_sha() const { if (!sha) sha = std::unique_ptr<Botan::HashFunction>(Botan::HashFunction::create("SHA-512")); }

        template <typename... Hashers>
        Botan::secure_vector<uint8_t> hash_self () {
            init_sha();
            for (const auto& part: parts) {
                sha->update(part.template hash_self<Hashers...>());
            }
            return sha->final();
        }

        Score fuse_tied_notes() const {
            auto sc = Score(work_title.c_str());
            for (const auto& part: parts) {
                sc.parts.push_back(part.fuse_tied_notes());
            }
            return sc;
        }

        friend std::ostream& operator<<(std::ostream& os, const Score& sc) {
            os << fmt::format("<Score[work_title={}]\n", sc.work_title);
            for (const auto& part : sc.parts) {
                os << part << std::endl;
            }
            os << ">";
            return os;
        }
    };

    template <
            typename OffsetT,
            typename DurationT,
            template < typename, typename... > class MeaCont = std::vector,
            template < typename, typename... > class GNoteCont = std::vector,
            template < typename, typename... > class MPartCont = std::vector
            >
    class MeasuredScore {
    public:
        using GNoteT = m32::GNote<OffsetT, DurationT, GNoteCont>;
        using MPartT = MeasuredPart<OffsetT, DurationT, GNoteT, MeaCont, GNoteCont>;
        std::string work_title;
        MPartCont<MPartT> measured_parts;
        std::unique_ptr<Botan::HashFunction> sha;
        using Self = MeasuredScore<OffsetT, DurationT, MeaCont, GNoteCont, MPartCont>;

        MeasuredScore() {}
        MeasuredScore(const char* work_title)
            : work_title(work_title)
            , sha { nullptr }
        {}
        MeasuredScore(const Self& other)
            : work_title(other.work_title)
            , measured_parts(other.measured_parts)
            , sha { nullptr }
        {}
        MeasuredScore& operator=(const Self& other) {
           work_title = other.work_title;
           measured_parts = other.measured_parts;
        }

        template <typename Arch>
        void serialize(Arch& arch) {
            arch(
                    CEREAL_NVP(work_title),
                    CEREAL_NVP(measured_parts)
            );
        }

        void clear() {
            work_title.clear();
            measured_parts.clear();
        }

        friend std::ostream& operator<<(std::ostream& os, const MeasuredScore& sc) {
            os << fmt::format("<MeasuredScore[work_title={}]\n", sc.work_title);
            for (const auto& part : sc.measured_parts) {
                os << part << std::endl;
            }
            os << ">";
            return os;
        }

        bool exact_eq(Self& other) {
            return work_title == other.work_title && measured_parts.size() == other.measured_parts.size() && std::equal(measured_parts.begin(), measured_parts.end(), other.measured_parts.begin(), [](const auto& a, const auto& b){ return a.exact_eq(b); });
        }
    };
}

#endif //AIDA_SCORE_H
