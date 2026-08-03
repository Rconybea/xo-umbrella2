set(SELF_LIB xo_distribution)
set(SELF_SRCS Normal.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} refcnt)
# public headers (ExplicitDist.hpp, ...) include <xo/indentlog/print/tostr.hpp>.
# Declared explicitly: xo-refcnt no longer propagates xo-indentlog.
xo_dependency(${SELF_LIB} indentlog)
