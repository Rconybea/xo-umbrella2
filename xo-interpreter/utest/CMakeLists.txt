# build unittest interpreter/utest

set(UTEST_EXE utest.interpreter)
set(UTEST_SRCS
    interpreter_utest_main.cpp
    StackFrame.test.cpp
)

xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
xo_self_dependency(${UTEST_EXE} xo_interpreter)
xo_dependency(${UTEST_EXE} xo_object)
xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
