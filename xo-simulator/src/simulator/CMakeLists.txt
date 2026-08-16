# xo-simulator/src/simulator/CMakeLists.txt

set(SELF_LIB simulator)
set(SELF_SRCS
    Simulator.cpp SourceTimestamp.cpp
    init_simulator.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

xo_dependency(${SELF_LIB} reactor)
# PUBLIC: Simulator.hpp:159 names xo::pp::scope in its API.
xo_dependency(${SELF_LIB} xo_ppsink)
# SourceTimestamp.cpp uses xo::pp::tostr; no public header needs indentlog2.
xo_dependency(${SELF_LIB} xo_indentlog2)
