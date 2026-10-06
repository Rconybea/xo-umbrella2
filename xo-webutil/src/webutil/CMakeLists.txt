# webutil/CMakeLists.txt

set(SELF_LIB webutil)
set(SELF_SRCS
	StreamReceiver.cpp
	StreamEndpointDescr.cpp
	HttpEndpointDescr.cpp
	HttpRequest.cpp
	HttpResponse.cpp
	Alist.cpp)

# reminder: can't be header-only library,  because depends on non-header-only callback (bc of non-header-only refcnt)
xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

xo_dependency(${SELF_LIB} refcnt)
# PUBLIC: StreamReceiver.hpp derives from reflect::SelfTagging
xo_dependency(${SELF_LIB} reflect)
xo_dependency(${SELF_LIB} callback)
# PUBLIC: StreamEndpointDescr.hpp / HttpEndpointDescr.hpp name xo::pp::PpSink
# and include Prettifier.hpp, so consumers need ppsink too.
xo_dependency(${SELF_LIB} xo_ppsink)
# StreamEndpointDescr.cpp / HttpEndpointDescr.cpp call xo::pp::tostr; no public
# header of xo-webutil needs indentlog2.
xo_dependency(${SELF_LIB} xo_indentlog2)

# end CMakeLists.txt
