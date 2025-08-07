# build unittest object/utest

set(UTEST_EXE utest.object)
set(UTEST_SRCS
    object_utest_main.cpp
    Boolean.test.cpp
    Integer.test.cpp
    String.test.cpp
    List.test.cpp
    GC.test.cpp)

xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
xo_self_dependency(${UTEST_EXE} xo_object)
xo_dependency(${UTEST_EXE} randomgen)
xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
