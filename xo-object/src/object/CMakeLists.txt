# object/CMakeLists.txt

set(SELF_LIB xo_object)
set(SELF_SRCS
    Boolean.cpp
    String.cpp
    List.cpp
    Integer.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} xo_alloc)
