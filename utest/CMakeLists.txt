# xo-ratio/utest/CMakeLists.txt

set(SELF_EXE utest.ratio)
set(SELF_SRCS
    ratio_utest_main.cpp
    ratio.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})

    xo_utest_coverage_config2()

    # ----------------------------------------------------------------
    # dependencies..

    xo_self_headeronly_dependency(${SELF_EXE} xo_ratio)
    xo_dependency(${SELF_EXE} randomgen)
    xo_dependency(${SELF_EXE} indentlog)
    xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
