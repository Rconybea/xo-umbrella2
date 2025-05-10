set(SELF_LIB refcnt)
set(SELF_SRCS Refcounted.cpp Displayable.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_install_include_tree3(include/xo/cxxutil)

# NOTE:
# dependency set here must be kept consistent with refcnt/cmake/refcntConfig.cmake.in
#
xo_dependency(${SELF_LIB} indentlog)
