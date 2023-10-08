set(SELF_LIB refcnt)
set(SELF_SRCS Refcounted.cpp Displayable.cpp)

xo_add_shared_library(${SELF_LIB} ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} indentlog)
