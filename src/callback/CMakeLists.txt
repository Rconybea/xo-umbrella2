# callback/CMakeLists.txt

set(SELF_LIB callback)
set(SELF_SRCS CallbackSet.cpp)

# reminder: can't be header-only library,  because depends on non-header-only refcnt
xo_add_shared_library(${SELF_LIB} ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies:

xo_dependency(${SELF_LIB} refcnt)

# end CMakeLists.txt
