# xo-simulator/src/simulator/CMakeLists.txt

set(SELF_LIB simulator)
set(SELF_SRCS
    Simulator.cpp SourceTimestamp.cpp
    init_simulator.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

# note: changes to xo_dependency() calls here
#       must coordinate with find_dependency() calls
#       in xo-simulator/cmake/simulatorConfig.cmake.in
#
xo_dependency(${SELF_LIB} reactor)
