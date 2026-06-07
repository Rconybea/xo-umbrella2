# built unittest xo-object2/utest

set(UTEST_EXE utest.stringtable2)
set(UTEST_SRCS
    stringtable2_utest_main.cpp
    StringTable.test.cpp
    DString.test.cpp
    StringOps.test.cpp
)

xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
xo_self_dependency(${UTEST_EXE} xo_stringtable2)
#xo_dependency(${UTEST_EXE} randomgen)
xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
