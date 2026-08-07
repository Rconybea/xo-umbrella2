# indentlog2/utest/CMakeLists.txt

# note: tests in this directory use catch2-provided main
set(SELF_EXE utest.indentlog2)
set(SELF_SOURCE_FILES
    indentlog2_utest_main.cpp
    LineState.test.cpp
    LogBuffer.test.cpp
    PpState.test.cpp
    put_with_escape.test.cpp
    PpTokenStreambuf.test.cpp
    PrettySink.test.cpp
    FlatSink.test.cpp
    Prettifier.test.cpp
    PrettyVector.test.cpp
    scope.test.cpp
    span_pp.test.cpp
    pretty_struct.test.cpp
    group_fit.test.cpp
    hex.test.cpp)

if (ENABLE_TESTING)
    # xo_add_utest_executable: add_executable + xo_include_options2 + add_test,
    # and registers ${SELF_EXE} with all_utest_executables_${PROJECT_NAME}
    # (and the global utest list)
    xo_add_utest_executable(${SELF_EXE} ${SELF_SOURCE_FILES})

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
