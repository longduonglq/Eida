set(COMMON_SRC
        common.h
        Interval.h
        generator.hpp
        recursive_generator.hpp
        tmp.cpp
        XmlSax.h gconfig.h gconfig.cpp)

find_package( LibXml2 REQUIRED )

add_library(common ${COMMON_SRC})

target_include_directories(common PUBLIC
        $<BUILD_INTERFACE:
        ${PROJECT_SOURCE_DIR}
        ${LIBXML2_INCLUDE_DIR}
        >
        )

target_link_libraries(common PRIVATE
        fmt
        ${LIBXML2_LIBRARY}
        )