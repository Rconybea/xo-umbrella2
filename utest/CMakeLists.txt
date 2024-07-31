# xo-parser/utest/CMakeLists.txt

set(UTEST_EXE utest.parser)
set(UTEST_SRCS
    parser_utest_main.cpp
    parser.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
    xo_self_dependency(${UTEST_EXE} xo_parser)
    #xo_dependency(${UTEST_EXE} xo_ratio)
    #xo_dependency(${UTEST_EXE} xo_reflectutil)
    xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
