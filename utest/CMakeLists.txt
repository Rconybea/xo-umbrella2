# build unittest distribution/utest

set(SELF_EXE utest.distribution)
set(SELF_SRCS
    distribution_utest_main.cpp
    Normal.test.cpp
    Uniform.test.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
target_code_coverage(${SELF_EXE} AUTO ALL)

xo_self_dependency(${SELF_EXE} xo_distribution)

xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
