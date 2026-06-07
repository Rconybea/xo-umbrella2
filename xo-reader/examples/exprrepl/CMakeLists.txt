# xo-reader/example/exprrepl/CMakeLists.txt

set(SELF_EXE xo_expression_repl)
set(SELF_SRCS exprrepl.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_self_dependency(${SELF_EXE} xo_reader)
endif()

# end CMakeLists.txt
