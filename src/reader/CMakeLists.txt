# parser/CMakeLists.txt

set(SELF_LIB xo_reader)
set(SELF_SRCS
    parser.cpp
    reader.cpp
    exprstate.cpp
    define_xs.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} xo_expression)
xo_dependency(${SELF_LIB} xo_tokenizer)

# end CMakeLists.txt
