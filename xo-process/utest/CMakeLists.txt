# build unittest 'process/unittest'

# These tests can use the Catch2-provided main

set(SELF_EXE utest.process)
set(SELF_SRCS
    ProcessReflect.test.cpp
    RealizationSource.test.cpp
    process_utest_main.cpp)

xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})
xo_self_dependency(${SELF_EXE} process)
xo_dependency(${SELF_EXE} simulator)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
