# object/CMakeLists.txt

set(SELF_LIB xo_object)
set(SELF_SRCS
    object.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} reflect)
#xo_boost_dependency(${SELF_LIB})

# end CMakeLists.txt
