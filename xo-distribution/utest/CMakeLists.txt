# build unittest distribution/utest

set(SELF_EXE utest.distribution)
set(SELF_SRCS
    distribution_utest_main.cpp
    Normal.test.cpp
    Uniform.test.cpp)

xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})
xo_self_dependency(${SELF_EXE} xo_distribution)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
