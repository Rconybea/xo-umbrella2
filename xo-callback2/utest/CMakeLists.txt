# xo-callback2/utest/CMakeLists.txt

set(SELF_EXE utest.callback2)
set(SELF_SRCS
    callback2_utest_main.cpp
    CallbackSet.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})

    xo_utest_coverage_config2()

    # ----------------------------------------------------------------
    # dependencies..

    xo_self_headeronly_dependency(${SELF_EXE} xo_callback2)
    # need explicit header-only dependencies, at least in submodule build
    xo_headeronly_dependency(${SELF_EXE} callback)
    xo_headeronly_dependency(${SELF_EXE} xo_facet)
    xo_dependency(${SELF_EXE} xo_alloc2)
    xo_dependency(${SELF_EXE} xo_ppsink)
    # UtestAppStart/UtestListener: shared utest main
    xo_dependency(${SELF_EXE} xo_testutil)
    xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
