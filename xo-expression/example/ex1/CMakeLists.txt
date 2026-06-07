# xo-expression/example/ex1/CMakeLists.txt

set(SELF_EXE xo_expression_ex1)
set(SELF_SRCS ex1.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_self_dependency(${SELF_EXE} xo_expression)
    xo_dependency(${SELF_EXE} refcnt)
endif()

# end CMakeLists.txt
