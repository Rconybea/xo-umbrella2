# xo-gc/utest/CMakeLists.txt
#

set(UTEST_EXE utest.gc)
set(UTEST_SRCS
    gc_utest_main.cpp
    Collector.test.cpp
    X1Collector.test.cpp
    DX1CollectorIterator.test.cpp
    MutationLogStore.test.cpp
    GCObjectStore.test.cpp
    GCObjectConversion.test.cpp
    Object2.test.cpp
    ObjectAge.test.cpp

    DMockCollector.cpp
    ICollector_DMockCollector.cpp
    MockCollector.test.cpp

    MlsTestutil.cpp
    GcosTestutil.cpp

    init_gc_utest.cpp
    random_allocs.cpp
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-gc-facetimpl-collector-mockcollector
    FACET_PKG xo_alloc2
    INPUT idl/ICollector_DMockCollector.json5
)

if (ENABLE_TESTING)
    xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
    xo_headeronly_dependency(${UTEST_EXE} randomgen)
    xo_self_dependency(${UTEST_EXE} xo_gc)
#    xo_headeronly_dependency(${UTEST_EXE} indentlog)
    xo_headeronly_dependency(${UTEST_EXE} xo_facet)
    xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
