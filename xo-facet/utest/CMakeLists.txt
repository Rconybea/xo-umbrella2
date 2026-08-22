# xo-facet/utest/CMakeLists.txt
#

set(UTEST_EXE utest.facet)
set(UTEST_SRCS
    facet_utest_main.cpp
    objectmodel.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
    xo_self_dependency(${UTEST_EXE} xo_facet)
    # UtestAppStart owns the single catch2 runtime (see facet_utest_main.cpp)
    xo_dependency(${UTEST_EXE} xo_testutil)
    # ThreadPrettySink: opt this suite in to pretty-printed scope output
    xo_dependency(${UTEST_EXE} xo_ppsink)
    xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
    xo_external_target_dependency(${UTEST_EXE} CLI11 CLI11::CLI11)
endif()

# end CMakeLists.txt
