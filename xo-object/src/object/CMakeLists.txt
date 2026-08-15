# object/CMakeLists.txt

set(SELF_LIB xo_object)
set(SELF_SRCS
    ObjectConverter.cpp
    Boolean.cpp
    String.cpp
    List.cpp
    Integer.cpp
    Float.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_headeronly_dependency(${SELF_LIB} xo_reflectutil)
xo_headeronly_dependency(${SELF_LIB} xo_unit)
xo_headeronly_dependency(${SELF_LIB} callback)
xo_dependency(${SELF_LIB} xo_alloc)
# PUBLIC: Boolean/Float/Integer/String.hpp use xo::pp::xtag in inline bodies,
# so consumers need ppsink too.
xo_dependency(${SELF_LIB} xo_ppsink)
# PUBLIC for the same reason: those headers also call xo::pp::tostr inline.
xo_dependency(${SELF_LIB} xo_indentlog2)
#xo_dependency(${SELF_LIB} reflect)  # should be able to get this indirectly via xo_alloc
