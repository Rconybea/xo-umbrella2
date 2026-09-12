# xo-reflectable2/src/reflectable2/CMakeLists.txt

set(SELF_LIB xo_reflectable2)
set(SELF_SRCS
    IReflectable_Any.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# input dependencies
#
# NB xo-reflectable2/cmake/xo_reflectable2Config.cmake.in generates its
#    find_dependency() calls from these, so there is no parallel list to keep
#    in step.

xo_dependency(${SELF_LIB} xo_facet)
xo_dependency(${SELF_LIB} reflect)

# end src/reflectable2/CMakeLists.txt
