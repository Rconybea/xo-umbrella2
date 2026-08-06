# webutil/CMakeLists.txt

set(SELF_LIB webutil)
set(SELF_SRCS StreamEndpointDescr.cpp HttpEndpointDescr.cpp Alist.cpp)

# reminder: can't be header-only library,  because depends on non-header-only callback (bc of non-header-only refcnt)
xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

xo_dependency(${SELF_LIB} refcnt)
xo_dependency(${SELF_LIB} callback)
# StreamEndpointDescr.cpp / HttpEndpointDescr.cpp use xo::pp::{xtag,tostr}.
# Only the .cpp files -- no public header of xo-webutil needs it.
xo_dependency(${SELF_LIB} xo_ppsink)

# end CMakeLists.txt
