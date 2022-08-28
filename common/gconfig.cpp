// Project AIDA
// Created by Long Duong on 7/15/22.
// Purpose: 
//
#include "gconfig.h"

namespace common {
    gconfig &gconfig::instance() {
        static gconfig inst;
        return inst;
    }

    gconfig::gconfig() {
        tbl = toml::parse_file(config_fn);
    }
}