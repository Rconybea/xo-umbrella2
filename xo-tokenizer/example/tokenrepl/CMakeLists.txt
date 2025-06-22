# xo-tokenizer/example/tokenrepl/CMakeLists.txt

set(SELF_EXE xo_tokenizer_repl)
set(SELF_SRCS tokenrepl.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_dependency(${SELF_EXE} xo_tokenizer)
endif()

# end CMakeLists.txt
