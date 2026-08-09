# xo-webutil/utest/CMakeLists.txt

set(SELF_EXE utest.webutil)
set(SELF_SRCS
    webutil_utest_main.cpp
    EndpointDescr.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})

    xo_self_dependency(${SELF_EXE} webutil)
    # for xo::pp::{FlatSink,tostr} in the tests themselves
    xo_dependency(${SELF_EXE} xo_ppsink)
    # UtestAppStart/UtestListener: shared utest main, see webutil_utest_main.cpp
    xo_dependency(${SELF_EXE} xo_testutil)
    xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
