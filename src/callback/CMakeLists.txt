# callback/CMakeLists.txt

set(SELF_LIB callback)
#set(SELF_SRCS CallbackSet.cpp)

xo_add_headeronly_library(${SELF_LIB})
xo_install_library3(${SELF_LIB} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# external dependencies:

#xo_dependency(${SELF_LIB} refcnt)

# end CMakeLists.txt
