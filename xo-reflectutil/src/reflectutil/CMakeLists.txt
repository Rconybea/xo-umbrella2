# reflectutil/CMakeLists.txt

set(SELF_LIB xo_reflectutil)
set(SELF_SRCS typeseq.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} xo_ppsink)

# end CMakeLists.txt
