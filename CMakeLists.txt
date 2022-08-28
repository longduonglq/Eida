cmake_minimum_required(VERSION 3.20)
project(aida)

set(CMAKE_CXX_STANDARD 23)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package( Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/tomlplusplus/include)
find_package( Catch2 REQUIRED )

find_package( fmt REQUIRED )

add_subdirectory(common)
add_subdirectory(m32)
add_subdirectory(tinynote)
add_subdirectory(db/mbank)
add_subdirectory(mash)
add_subdirectory(mash/mini32)
add_subdirectory(mash/heval)

add_executable(aida main.cpp)
target_link_libraries(aida
        common
        m32
        ${Boost_LIBRARIES}
        Catch2::Catch2
        fmt
)


add_subdirectory(tests)
add_subdirectory(engra)
add_subdirectory(thirdparty/libsimdpp)