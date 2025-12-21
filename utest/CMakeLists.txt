# xo-alloc2/utest/CMakeLists.txt
#

set(UTEST_EXE utest.alloc2)
set(UTEST_SRCS
    alloc2_utest_main.cpp
    objectmodel.test.cpp
    arena.test.cpp
    DArenaIterator.test.cpp
    Collector.test.cpp
    DX1CollectorIterator.test.cpp
    random_allocs.cpp
)

if (ENABLE_TESTING)
    xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
    xo_headeronly_dependency(${UTEST_EXE} randomgen)
    xo_self_dependency(${UTEST_EXE} xo_alloc2)
    xo_headeronly_dependency(${UTEST_EXE} indentlog)
    xo_headeronly_dependency(${UTEST_EXE} xo_facet)
    xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
