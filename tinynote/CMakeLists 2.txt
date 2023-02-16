set(TINYNOTE_SRC
        PitchParser.h
        SimpleNoteParser.h
        TupletParser.h
        MeasureParser.h
        ParseException.h
        common.h
        tmp.cpp
        Number.h
        Length.h
        PartParser.h)

add_library(tinynote ${TINYNOTE_SRC})
target_link_libraries(tinynote PRIVATE
        m32
        fmt)

#target_include_directories(tinynote
#
#        PUBLIC
#        $<BUILD_INTERFACE:
#            ${CMAKE_SOURCE_DIR}/thirdparty/PEGTL/include
#        >
#        )