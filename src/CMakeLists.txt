set(SELF_LIBRARY_NAME refcnt)
set(SELF_SOURCE_FILES Refcounted.cpp Displayable.cpp)

xo_add_shared_library(${SELF_LIBRARY_NAME} ${PROJECT_VERSION} 1 ${SELF_SOURCE_FILES})
xo_internal_dependency(${SELF_LIBRARY_NAME} indentlog)
