# build unittest expression/utest

set(SELF_EXE utest.expression)
set(SELF_SRCS
    expression_utest_main.cpp
    type_unifier.test.cpp)

xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})
xo_self_dependency(${SELF_EXE} xo_expression)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
