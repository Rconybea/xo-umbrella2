# xo-reflectable2/utest/CMakeLists.txt

set(UTEST_EXE utest.reflectable2)
set(UTEST_SRCS
    reflectable2_utest_main.cpp
    FopTdx.test.cpp
)

if (ENABLE_TESTING)
    xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
    xo_self_dependency(${UTEST_EXE} xo_reflectable2)
    xo_dependency(${UTEST_EXE} xo_testutil)
    xo_dependency(${UTEST_EXE} reflect)
    xo_dependency(${UTEST_EXE} xo_facet)
    xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
    xo_external_target_dependency(${UTEST_EXE} CLI11 CLI11::CLI11)
endif()

# end CMakeLists.txt
