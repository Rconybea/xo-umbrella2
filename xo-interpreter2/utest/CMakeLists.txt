# build unittest xo-interpreter2/utest

set(UTEST_EXE utest.interpreter2)
set(UTEST_SRCS
    interpreter2_utest_main.cpp
    VirtualSchematikaMachine.test.cpp
    printable_render.test.cpp
)

xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
xo_self_dependency(${UTEST_EXE} xo_interpreter2)
# UtestRehearser (two-pass harness), PrettySink via toppstr
xo_dependency(${UTEST_EXE} xo_testutil)
xo_dependency(${UTEST_EXE} xo_indentlog2)
xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
