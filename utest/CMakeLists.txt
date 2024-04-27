# xo-unit/utest/CMakeLists.txt

set(SELF_EXECUTABLE_NAME utest.unit)
set(SELF_SOURCE_FILES
    unit_utest_main.cpp  #mpl_unit.test.cpp
    unit.test.cpp #quantity.test.cpp
)

xo_add_utest_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})

# ----------------------------------------------------------------
# dependencies..

xo_self_dependency(${SELF_EXECUTABLE_NAME} xo_unit)
xo_external_target_dependency(${SELF_EXECUTABLE_NAME} Catch2 Catch2::Catch2)

# end CMakeLists.txt
