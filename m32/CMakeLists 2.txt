find_package(folly REQUIRED)

set(M32_SOURCES
        SimpleNote.h
        Pitch.cpp
        Pitch.h
        GNote.h
        Lyric.cpp
        Lyric.h
        Color.h
        types.h
        TieInfo.h
        utility.h
        Duration.cpp
        Duration.h
        Tuplet.cpp
        Tuplet.h
        M32Except.h

        Measure.h
        Part.h
        cmps.h
        debugprint.cpp
        debugprint.h
        Score.h
        MxlImport.h
        MxlExport.h
        show.h MidiExport.h MidiImport.h MidiImport.cpp Stream.h)

add_library(m32 ${M32_SOURCES})

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
find_package( Botan REQUIRED )
# find_package( LibXml2 REQUIRED )

set_target_properties(m32 PROPERTIES LINKER_LANGUAGE CXX)
include_directories(${PROJECT_SOURCE_DIR}/common)
# include_directories(${folly_INCLUDE_DIRS})

target_include_directories(m32 PUBLIC
        $<BUILD_INTERFACE:
                ${PROJECT_SOURCE_DIR}
                ${BOTAN_INCLUDE_DIRS}
                # ${LIBXML2_INCLUDE_DIR}
                ${PROJECT_SOURCE_DIR}/thirdparty/rapidxml
        >)

target_link_libraries(
        m32
        common
        fmt
        ${BOTAN_LIBRARIES}
        ${Boost_LIBRARIES}
        # ${LIBXML2_LIBRARY}
        # Folly::folly
)

# for some reason, fmt needs this macro
add_compile_definitions(FMT_HEADER_ONLY)