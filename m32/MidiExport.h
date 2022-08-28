// Project AIDA
// Created by Long Duong on 7/16/22.
// Purpose: 
//

#ifndef AIDA_MIDIEXPORT_H
#define AIDA_MIDIEXPORT_H

#include <exception>
#include <string_view>
#include <fmt/format.h>

namespace m32 {
class MidiExportException : public std::exception {
public:
    template <typename ... Args>
    explicit MidiExportException(std::string_view fmt, Args... args)
    : msg (fmt::vformat(fmt, fmt::make_format_args(args...)))
    {}
    virtual const char* what() const noexcept {return msg.c_str();}
private:
    std::string msg;
};

template <typename ScoreT>
struct MidiExporter {

};

}

#endif //AIDA_MIDIEXPORT_H
