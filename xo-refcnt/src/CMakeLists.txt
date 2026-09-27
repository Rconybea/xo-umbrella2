set(SELF_LIB refcnt)
set(SELF_SRCS Refcounted.cpp Displayable.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_install_include_tree3(include/xo/cxxutil)

# NOTE:
# dependency set here must be kept consistent with refcnt/cmake/refcntConfig.cmake.in
#
xo_dependency(${SELF_LIB} xo_reflectutil)
xo_dependency(${SELF_LIB} xo_ppsink)
# NOTE: deliberately no xo-indentlog dependency.  No refcnt source or header
# needs it (the opt-in compatibility headers pretty_refcnt.hpp and
# Refcounted_indentlog.hpp, which included legacy indentlog, are deleted:
# nothing included them).  Propagating it from here (xo_dependency is
# PUBLIC) put indentlog on the command line of every refcnt consumer, which
# masked whether downstream code had really migrated.
