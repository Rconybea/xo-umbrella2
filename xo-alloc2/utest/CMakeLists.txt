# xo-alloc2/utest/CMakeLists.txt
#

set(UTEST_EXE utest.alloc2)
set(UTEST_SRCS
    alloc2_utest_main.cpp
    objectmodel.test.cpp
    arena.test.cpp
    IAllocator_Any.test.cpp
    DArenaIterator.test.cpp
    Generation.test.cpp
    Role.test.cpp
    VisitReason.test.cpp
    ResourceVisitor.test.cpp
    dp.test.cpp
    random_allocs.cpp
)

if (ENABLE_TESTING)
    xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
    xo_self_dependency(${UTEST_EXE} xo_alloc2)
    xo_dependency(${UTEST_EXE} xo_testutil)
    xo_headeronly_dependency(${UTEST_EXE} randomgen)
    xo_headeronly_dependency(${UTEST_EXE} indentlog)
    xo_headeronly_dependency(${UTEST_EXE} xo_facet)
    xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
    xo_external_target_dependency(${UTEST_EXE} CLI11 CLI11::CLI11)
endif()

# end CMakeLists.txt
