# xo-facet/utest/CMakeLists.txt
#

set(UTEST_EXE utest.facet)
set(UTEST_SRCS
    facet_utest_main.cpp
    objectmodel.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${UTEST_EXE} ${UTEST_SRCS})
    xo_external_target_dependency(${UTEST_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
