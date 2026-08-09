# built unittest xo-object2/utest

set(UTEST_EXE utest.object2)
set(UTEST_SRCS
    object2_utest_main.cpp
    DArray.test.cpp
    printable_render.test.cpp
#    DString.test.cpp
#    StringOps.test.cpp
#    X1Collector.test.cpp   # moved to xo-gc/
#   Printable.test.cpp  # moved to xo-gc/
)

xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
xo_self_dependency(${UTEST_EXE} xo_object2)
# UtestRehearser (two-pass harness), PrettySink via toppstr
xo_dependency(${UTEST_EXE} xo_testutil)
xo_dependency(${UTEST_EXE} xo_indentlog2)
xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
