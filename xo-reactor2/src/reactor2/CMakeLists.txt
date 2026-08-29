# xo-reactor2/src/reactor2/CMakeLists.txt

set(SELF_LIB xo_reactor2)
set(SELF_SRCS
    init_reactor2.cpp
    setup_reactor2.cpp
    IAbstractEventProcessor_Any.cpp
    IEventSink_Any.cpp
    IEventSource_Any.cpp
    IGCObject_DObjectEvent.cpp
    IPrintable_DObjectEvent.cpp
    DObjectEvent.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# input dependencies
#
# NOTE: dependency set here must be kept consistent with
#       xo-reactor2/cmake/xo_reactor2Config.cmake.in

xo_dependency(${SELF_LIB} callback)
xo_dependency(${SELF_LIB} xo_stringtable2)
xo_dependency(${SELF_LIB} reflect)
xo_dependency(${SELF_LIB} xo_facet)

# end src/reactor2/CMakeLists.txt
