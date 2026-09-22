# xo-reflectutil/utest/CMakeLists.txt

set(SELF_EXE utest.reflectutil)
set(SELF_SRCS
    reflectutil_utest_main.cpp
    typeseq.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})

    xo_self_dependency(${SELF_EXE} xo_reflectutil)
    xo_dependency(${SELF_EXE} xo_ppsink)
    xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
