// Project AIDA
// Created by Long Duong on 7/15/22.
// Purpose: 
//

#ifndef AIDA_SHOW_H
#define AIDA_SHOW_H

#include "MxlExport.h"
#include "types.h"
#include "SimpleNote.h"
#include "Tuplet.h"
#include "GNote.h"
#include "Part.h"
#include "Score.h"
#include "common/gconfig.h"

#include <type_traits>

#include <boost/filesystem.hpp>
#include <fmt/format.h>

using namespace m32;

namespace m32::show {
    using simple_note_t = m32::SimpleNote<m32::Offset, m32::Duration>;
    using tuplet_t = m32::Tuplet<m32::Offset, m32::Duration>;
    using gnote_t = m32::GNote<m32::Offset, m32::Duration>;
    using part_t = m32::Part<gnote_t>;
    using mpart_t = m32::MeasuredPart<m32::Offset, m32::Duration, gnote_t>;
    using score_t = m32::Score<gnote_t>;
    using mscore_t = m32::MeasuredScore<m32::Offset, m32::Duration>;
    using exporter_t = m32::MxlExporter<mscore_t, false>;

    template <typename M32T>
    void show(const M32T& m32_element) {
        const auto& config = common::gconfig::instance();
        boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
        temp.replace_extension(".musicxml");

        exporter_t exporter = exporter_t();

        if constexpr(std::is_same_v<M32T, mscore_t>) {
            exporter.xml_to_path(temp.c_str(), m32_element);
        }
        else if constexpr(std::is_same_v<M32T, mpart_t>) {
            mscore_t mscore("temp");
            mscore.measured_parts.push_back(m32_element);
            exporter.xml_to_path(temp.c_str(), mscore);
        }
        else if constexpr(std::is_same_v<M32T, simple_note_t>) {
            mpart_t mpart("temp-part", KeySigType(0), ClefSignType('G'), TimeSigType(4, 4));
            mpart.append_simple_note(m32_element);
            return show(mpart);
        }
        else if constexpr(std::is_same_v<M32T, tuplet_t>) {
            mpart_t mpart("temp-part", KeySigType(0), ClefSignType('G'), TimeSigType(4, 4));
            mpart.append_gnote(m32_element);
            return show(mpart);
        }
        else if constexpr(std::is_same_v<M32T, gnote_t>) {
            mpart_t mpart("temp-part", KeySigType(0), ClefSignType('G'), TimeSigType(4, 4));
            mpart.append_gnote(m32_element);
            return show(mpart);
        }

        std::system(
                fmt::format("{} {} {}",
                config.tbl["musescore"]["open_cmd"].template value<std::string_view>().value(),
                config.tbl["musescore"]["executable_path"].template value<std::string_view>().value(),
                temp.c_str()).c_str());
    }

    void inline show(std::vector<gnote_t> gns) {
        mpart_t mpart("temp-part", KeySigType(0), ClefSignType('G'), TimeSigType(4, 4));
        for (auto& gn : gns) mpart.append_gnote(gn);
        show(mpart);
    }
}

#endif //AIDA_SHOW_H
