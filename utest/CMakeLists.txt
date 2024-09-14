# build unittest reactor/unittest'

set(SELF_EXE utest.reactor)
set(SELF_SRCS Sink.test.cpp PollingReactor.test.cpp reactor_utest_main.cpp)

xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})
xo_self_dependency(${SELF_EXE} reactor)
xo_dependency(${SELF_EXE} randomgen)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
