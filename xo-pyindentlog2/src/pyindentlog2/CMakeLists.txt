# xo_pyindentlog2/src/pyindentlog2/CMakeLists.txt

set(SELF_LIB xo_pyindentlog2)
set(SELF_SRCS pyindentlog2.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
# pyutil.hpp includes <xo/refcnt/Refcounted.hpp>; xo_pyutil is an INTERFACE
# target so its own deps do not reach us (cf. xo-pyunit, same pattern)
xo_pybind11_dependency(${SELF_LIB} refcnt)
xo_pybind11_dependency(${SELF_LIB} xo_indentlog2)
# ArenaConfig/AllocHeaderConfig are xo-arena types, included directly here --
# declare the edge rather than leaning on xo_indentlog2 transitively
xo_pybind11_dependency(${SELF_LIB} xo_arena)

# end CMakeLists.txt
