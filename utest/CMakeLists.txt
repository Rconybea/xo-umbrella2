# build unittest 'process/unittest'

# These tests can use the Catch2-provided main

set(SELF_EXE utest.process)
set(SELF_SRCS
    ProcessReflect.test.cpp
    RealizationSource.test.cpp
    process_utest_main.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
target_code_coverage(${SELF_EXE} AUTO ALL)

# ----------------------------------------------------------------
# internal dependencies (on this codebase)

xo_self_dependency(${SELF_EXE} process)

# ----------------------------------------------------------------
# external dependencies

xo_dependency(${SELF_EXE} simulator)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
