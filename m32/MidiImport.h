// Project AIDA
// Created by Long Duong on 7/16/22.
// Purpose: 
//

#ifndef AIDA_MIDIIMPORT_H
#define AIDA_MIDIIMPORT_H

namespace m32 {
    struct Midi {
        static Midi from_file(const char* path);
    };
}

#endif //AIDA_MIDIIMPORT_H
