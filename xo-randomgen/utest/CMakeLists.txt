# xo-randomgen/utest/CMakeLists.txt

set(SELF_EXE utest.randomgen)
set(SELF_SRCS
    randomgen_utest_main.cpp
    gaussianpair.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})

    xo_utest_coverage_config2()

    # ----------------------------------------------------------------
    # dependencies..

    xo_self_headeronly_dependency(${SELF_EXE} randomgen)
    xo_dependency(${SELF_EXE} xo_ppsink)
    # UtestAppStart/UtestListener: shared utest main, see randomgen_utest_main.cpp
    xo_dependency(${SELF_EXE} xo_testutil)
    xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
