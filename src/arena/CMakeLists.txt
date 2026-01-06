# xo-arena/src/CMakeLists.txt

set(SELF_LIB xo_arena)
set(SELF_SRCS
    AllocError.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_install_include_tree3(include/xo/arena)

# ----------------------------------------------------------------
# input dependencies
#
# NOTE: dependency set here must be kept consistent with
#       xo-arena/cmake/xo_arenaConfig.cmake.in

xo_dependency(${SELF_LIB} xo_reflectutil)

# end src/CMakeLists.txt
