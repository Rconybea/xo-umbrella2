set(SELF_LIB xo_distribution)
set(SELF_SRCS Normal.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} refcnt)
# PUBLIC: ExplicitDist.hpp / KolmogorovSmirnov.hpp / StdEmpirical.hpp name
# xo::pp::{scope,xtag} in inline bodies, so consumers need ppsink too.
xo_dependency(${SELF_LIB} xo_ppsink)
# PUBLIC for the same reason: the first two also call xo::pp::tostr inline.
xo_dependency(${SELF_LIB} xo_indentlog2)
