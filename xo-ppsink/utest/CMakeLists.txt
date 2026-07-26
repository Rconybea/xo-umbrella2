# xo-ppsink/utest/CMakeLists.txt

set(SELF_EXECUTABLE_NAME utest.ppsink)
set(SELF_SOURCE_FILES
    ppsink_utest_main.cpp
    FlatSink.test.cpp
    Pretty.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})
    xo_self_dependency(${SELF_EXECUTABLE_NAME} xo_ppsink)
    xo_external_target_dependency(${SELF_EXECUTABLE_NAME} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
