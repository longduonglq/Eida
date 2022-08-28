// Project AIDA
// Created by Long Duong on 7/15/22.
// Purpose: 
//

#ifndef AIDA_GCONFIG_H
#define AIDA_GCONFIG_H

#include <thread>
#include <toml++/toml.h>

namespace common {
    struct gconfig {
        const char* config_fn = "aida.toml";
        toml::table tbl;

        static gconfig& instance();
    private:
        gconfig() ;
    };
}

#endif //AIDA_GCONFIG_H
