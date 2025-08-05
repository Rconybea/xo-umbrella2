# build unittest object/utest

set(SELF_EXE utest.object)
set(SELF_SRCS
    object_utest_main.cpp
    String.test.cpp
    List.test.cpp
    GC.test.cpp)

xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})
xo_self_dependency(${SELF_EXE} xo_object)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
