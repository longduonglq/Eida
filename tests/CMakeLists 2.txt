project(tests)

find_package( Catch2 REQUIRED )
find_package( fmt REQUIRED )

add_executable(common_tests
        common/CommonMain.cpp
        common/IntervalTest.cpp)
target_include_directories(common_tests PRIVATE
        ${CMAKE_SOURCE_DIR})
target_link_libraries(common_tests
        Catch2::Catch2)

add_compile_definitions(TESTS_DIR="${PROJECT_SOURCE_DIR}")
# add_compile_definitions(SHOW_CMP_MISMATCH)

add_executable(m32_tests
        m32/M32Main.cpp
        m32/ColorTest.cpp
        m32/PitchTest.cpp
        m32/DurationTest.cpp
        m32/GNoteTest.cpp
        m32/SimpleNoteTest.cpp
        m32/TupletTest.cpp
        m32/MeasureTest.cpp
        m32/PartTest.cpp
        m32/ImportExport.cpp m32/MidiImportExport.cpp)
target_include_directories(
        m32_tests PRIVATE
        ${CMAKE_SOURCE_DIR})
target_link_libraries(m32_tests
        Catch2::Catch2
        m32
        tinynote)



add_executable(tinynote_tests
        tinynote/TinynoteMain.cpp
        tinynote/SingleNoteTest.cpp
        tinynote/pitchTest.cpp
        tinynote/CommonTest.cpp
        tinynote/TupletTest.cpp
        tinynote/Number.cpp
        tinynote/Length.cpp
        tinynote/MeasureTest.cpp
        tinynote/PartParser.cpp)
target_include_directories(tinynote_tests PRIVATE
        ${CMAKE_SOURCE_DIR})
target_link_libraries(tinynote_tests
        Catch2::Catch2
        tinynote
)
# for some odd reasons, fmt needs this
add_compile_definitions(FMT_HEADER_ONLY)


add_executable(db_mbank_test
        db/mbank/MbankMain.cpp
        db/mbank/InitTest.cpp)
target_include_directories(db_mbank_test PRIVATE
        ${CMAKE_SOURCE_DIR})
target_link_libraries(db_mbank_test
        Catch2::Catch2
        mbank
        )



add_compile_definitions(
        SIMDPP_64_BITS
        SIMDPP_USE_NULL=0

        SIMDPP_ARM=1
        SIMDPP_USE_NEON=1
        SIMDPP_USE_NEON_FLT_SP=1

        SIMDPP_ARCH_ARM_NEON_FLT_SP=1
        )
add_compile_definitions(M_ALIGN=16)

add_executable(mini32_test
        mash/mini32/Mini32Main.cpp
        mash/mini32/IntervalStream.cpp
        )
target_include_directories(mini32_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(mini32_test
        Catch2::Catch2
        mini32)


add_executable(
        mash_test
        mash/MashMain.cpp
        mash/DFSAlignTest.cpp
)
target_link_libraries(
        mash_test
        Catch2::Catch2
        mini32
        mash
)


add_executable(
        BitPcC
        mash/heval/HevalMain.cpp
        mash/heval/BitPcC.cpp
)
target_link_libraries(
        BitPcC
        Catch2::Catch2
        mini32
        mash
)