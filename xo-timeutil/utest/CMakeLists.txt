# xo-timeutil unit test

set(SELF_EXECUTABLE_NAME utest.timeutil)
set(SELF_SOURCE_FILES
    timeutil_utest_main.cpp
    timeutil.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})
    xo_self_dependency(${SELF_EXECUTABLE_NAME} xo_timeutil)
    xo_external_target_dependency(${SELF_EXECUTABLE_NAME} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
