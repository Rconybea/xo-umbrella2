set(SELF_LIBRARY_NAME refcnt)
set(SELF_SOURCE_FILES Refcounted.cpp Displayable.cpp)
add_library(${SELF_LIBRARY_NAME} SHARED ${SELF_SOURCE_FILES})

set_target_properties(${SELF_LIBRARY_NAME}
              PROPERTIES
              VERSION ${PROJECT_VERSION}
              SOVERSION 1)

xo_indentlog_dependency(${SELF_LIBRARY_NAME})

xo_include_options(${SELF_LIBRARY_NAME})
xo_compile_options(${SELF_LIBRARY_NAME})
xo_install_library(${SELF_LIBRARY_NAME})
