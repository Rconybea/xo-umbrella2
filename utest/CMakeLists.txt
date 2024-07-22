# build unittest tokenizer/utest

set(SELF_EXECUTABLE_NAME utest.tokenizer)
set(SELF_SOURCE_FILES
    tokenizer_utest_main.cpp
    tokenizer.test.cpp
    token.test.cpp)

xo_add_utest_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})
xo_self_dependency(${SELF_EXECUTABLE_NAME} tokenizer)
xo_external_target_dependency(${SELF_EXECUTABLE_NAME} Catch2 Catch2::Catch2)

# end CMakeLists.txt
