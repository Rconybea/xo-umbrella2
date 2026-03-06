# built unittest xo-type/utest

set(UTEST_EXE utest.type)
set(UTEST_SRCS
    type_utest_main.cpp
    DAtomicType.test.cpp
    DListType.test.cpp
    DArrayType.test.cpp
#    DString.test.cpp
#    StringOps.test.cpp
#    X1Collector.test.cpp
#    Printable.test.cpp
)

xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
xo_self_dependency(${UTEST_EXE} xo_type)
#xo_dependency(${UTEST_EXE} xo_alloc2)
xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
