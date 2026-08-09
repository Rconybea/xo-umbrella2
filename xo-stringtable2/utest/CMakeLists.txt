# built unittest xo-object2/utest

set(UTEST_EXE utest.stringtable2)
set(UTEST_SRCS
    stringtable2_utest_main.cpp
    StringTable.test.cpp
    DString.test.cpp
    StringOps.test.cpp
    printable_render.test.cpp
)

xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
xo_self_dependency(${UTEST_EXE} xo_stringtable2)
# UtestRehearser -- two-pass test harness, see printable_render.test.cpp
xo_dependency(${UTEST_EXE} xo_testutil)
# PrettySink (via toppstr) for the new-protocol renderings
xo_dependency(${UTEST_EXE} xo_indentlog2)
#xo_dependency(${UTEST_EXE} randomgen)
xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
