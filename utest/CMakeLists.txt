# build unittest reflect/utest

set(SELF_EXECUTABLE_NAME utest.reflect)
set(SELF_SOURCE_FILES reflect_utest_main.cpp StructReflector.test.cpp VectorTdx.test.cpp StructTdx.test.cpp)

xo_add_utest_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})

# ----------------------------------------------------------------
# deps: indentlog, ..

xo_self_dependency(${SELF_EXECUTABLE_NAME} reflect)
xo_external_target_dependency(${SELF_EXECUTABLE_NAME} Catch2 Catch2::Catch2)

# end CMakeLists.txt
