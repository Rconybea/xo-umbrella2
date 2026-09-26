# xo-reactor2websock/utest/CMakeLists.txt

set(SELF_EXE utest.reactor2websock)
set(SELF_SRCS
    reactor2websock_utest_main.cpp
    reactor2websock.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})

    xo_self_dependency(${SELF_EXE} reactor2websock)
    # UtestAppStart/UtestListener: shared utest main, see reactor2websock_utest_main.cpp
    xo_dependency(${SELF_EXE} xo_testutil)
    xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
