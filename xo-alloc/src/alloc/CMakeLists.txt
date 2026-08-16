# alloc/CMakeLists.txt

set(SELF_LIB xo_alloc)
set(SELF_SRCS
    ArenaAlloc.cpp
    ListAlloc.cpp
    GC.cpp
    GcStatistics.cpp
    ObjectStatistics.cpp
    Object.cpp
    Blob.cpp
    Forwarding1.cpp
    generation.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_headeronly_dependency(${SELF_LIB} xo_allocutil)
# xo-unit used for time measurement
xo_headeronly_dependency(${SELF_LIB} xo_unit)
# PUBLIC: CircularBuffer.hpp names xo::pp::{scope,xtag,tag} in inline bodies.
xo_dependency(${SELF_LIB} xo_ppsink)
# PUBLIC for the same reason: CircularBuffer.hpp also calls xo::pp::tostr inline.
xo_dependency(${SELF_LIB} xo_indentlog2)
xo_dependency(${SELF_LIB} reflect)
xo_headeronly_dependency(${SELF_LIB} callback)

#end CMakeLists.txt
