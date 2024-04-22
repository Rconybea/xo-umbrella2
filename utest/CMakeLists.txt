# xo-unit/utest/CMakeLists.txt

set(SELF_EXECUTABLE_NAME utest.unit)
set(SELF_SOURCE_FILES
    unit_utest_main.cpp
    mpl_unit.test.cpp
    unit.test.cpp quantity.test.cpp)

add_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})
xo_include_options2(${SELF_EXECUTABLE_NAME})

add_test(NAME ${SELF_EXECUTABLE_NAME} COMMAND ${SELF_EXECUTABLE_NAME})
target_code_coverage(${SELF_EXECUTABLE_NAME} AUTO ALL)

# ----------------------------------------------------------------
# internal dependencies: logutil, ...

xo_self_dependency(${SELF_EXECUTABLE_NAME} xo_unit)
xo_dependency(${SELF_EXECUTABLE_NAME} reflect)

# ----------------------------------------------------------------
# 3rd party dependency: catch2:

xo_external_target_dependency(${SELF_EXECUTABLE_NAME} Catch2 Catch2::Catch2)

# end CMakeLists.txt
