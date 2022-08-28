// Project AIDA
// Created by Long Duong on 7/16/22.
// Purpose: 
//

#include <catch2/catch.hpp>
#include <boost/filesystem.hpp>
#include "m32/MidiImport.h"

using namespace m32;
using namespace boost::filesystem;

TEST_CASE("midi-import") {
    boost::filesystem::path mxl_path(TESTS_DIR);
    mxl_path /= "m32/mxl";

    auto mid = Midi::from_file((mxl_path / "example2.mid").c_str());

}