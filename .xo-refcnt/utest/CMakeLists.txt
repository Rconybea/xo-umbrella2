# build unittest 'refcnt/utest/utest.refcnt

set(SELF_EXECUTABLE_NAME utest.refcnt)
# These tests can use the Catch2-provided main
set(SELF_SOURCE_FILES intrusive_ptr.test.cpp refcnt_utest_main.cpp)

xo_add_utest_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})

# ----------------------------------------------------------------
# 3rd party dependency: catch2:

xo_self_dependency(${SELF_EXECUTABLE_NAME} refcnt)
xo_external_target_dependency(${SELF_EXECUTABLE_NAME} Catch2 Catch2::Catch2)

# end CMakeLists.txt
