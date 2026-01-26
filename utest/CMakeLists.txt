# built unittest xo-procedure2/utest

set(UTEST_EXE utest.procedure2)
set(UTEST_SRCS
    procedure2_utest_main.cpp
    DPrimitive.test.cpp
)

xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
xo_self_dependency(${UTEST_EXE} xo_procedure2)
xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
