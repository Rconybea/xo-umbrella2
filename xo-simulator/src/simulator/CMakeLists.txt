# xo-simulator/src/simulator/CMakeLists.txt

set(SELF_LIB simulator)
set(SELF_SRCS
    Simulator.cpp SourceTimestamp.cpp
    init_simulator.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

xo_dependency(${SELF_LIB} reactor)
xo_dependency(${SELF_LIB} xo_ppsink)
