# indentlog2/utest/CMakeLists.txt

# note: tests in this directory use catch2-provided main
set(SELF_EXE utest.indentlog2)
set(SELF_SOURCE_FILES
    indentlog2_utest_main.cpp
    LineState.test.cpp)

if (ENABLE_TESTING)
    add_executable(${SELF_EXE} ${SELF_SOURCE_FILES})
    xo_include_options2(${SELF_EXE})

    add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})

    # ----------------------------------------------------------------
    # internal deps

    xo_self_dependency(${SELF_EXE} xo_indentlog2)
    xo_dependency(${SELF_EXE} randomgen)
    xo_dependency(${SELF_EXE} xo_testutil)

    # ----------------------------------------------------------------
    # external deps

    xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
    xo_external_target_dependency(${SELF_EXE} CLI11 CLI11::CLI11)
endif()

# end CMakeLists.txt
