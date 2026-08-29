# xo-reactor2/utest/CMakeLists.txt

set(UTEST_EXE utest.reactor2)
set(UTEST_SRCS
    reactor2_utest_main.cpp
    DObjectEvent.test.cpp
)

if (ENABLE_TESTING)
    xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})

    xo_utest_coverage_config2()

    xo_self_dependency(${UTEST_EXE} xo_reactor2)
    # DString: the obj<AGCObject> payload under test
    xo_dependency(${UTEST_EXE} xo_stringtable2)
    # APrintable facet, pivoted to by DObjectEvent::pretty()
    xo_dependency(${UTEST_EXE} xo_printable2)
    # arena + AGCObject
    xo_dependency(${UTEST_EXE} xo_alloc2)
    # PrettySink, via toppstr
    xo_dependency(${UTEST_EXE} xo_indentlog2)
    # UtestRehearser -- two-pass test harness
    xo_dependency(${UTEST_EXE} xo_testutil)
    xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
