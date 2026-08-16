# tokenizer/CMakeLists.txt

set(SELF_LIB xo_tokenizer)
set(SELF_SRCS
    tokentype.cpp
    token.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
# PUBLIC: input_state.hpp / span.hpp / tokenizer_error.hpp / tokentype.hpp name
# xo::pp::{scope,xtag,quot,unq,STRINGIFY} in inline bodies.
xo_dependency(${SELF_LIB} xo_ppsink)
# PUBLIC too: token.hpp / tokenizer.hpp call xo::pp::tostr inline.
xo_dependency(${SELF_LIB} xo_indentlog2)

# end CMakeLists.txt
