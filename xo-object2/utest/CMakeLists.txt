# built unittest xo-object2/utest

set(UTEST_EXE utest.object2)
set(UTEST_SRCS
    object2_utest_main.cpp
    X1Collector.test.cpp
)

xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
xo_self_dependency(${UTEST_EXE} xo_object2)
#xo_dependency(${UTEST_EXE} randomgen)
xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
