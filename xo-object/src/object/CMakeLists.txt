# object/CMakeLists.txt

set(SELF_LIB xo_object)
set(SELF_SRCS
    ObjectConverter.cpp
    Boolean.cpp
    String.cpp
    List.cpp
    Integer.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_headeronly_dependency(${SELF_LIB} xo_reflectutil)
xo_headeronly_dependency(${SELF_LIB} xo_unit)
xo_headeronly_dependency(${SELF_LIB} callback)
xo_dependency(${SELF_LIB} xo_alloc)
#xo_dependency(${SELF_LIB} reflect)  # should be able to get this indirectly via xo_alloc
