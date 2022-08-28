// Project AIDA
// Created by Long Duong on 7/16/22.
// Purpose: 
//

#include <iostream>
#include <fmt/format.h>
#include <fstream>
#include <types.h>

#include "MidiImport.h"

namespace m32 {
    Midi Midi::from_file(const char *path) {
        std::ifstream file(path);
        auto midi = Midi();

        return midi;
    }
}