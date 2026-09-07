# xo_pyarena/src/pyarena/CMakeLists.txt

set(SELF_LIB xo_pyarena)
set(SELF_SRCS pyarena.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
# pyutil.hpp includes <xo/refcnt/Refcounted.hpp>; xo_pyutil is an INTERFACE
# target so its own deps do not reach us (cf. xo-pyindentlog2, same pattern)
xo_pybind11_dependency(${SELF_LIB} refcnt)
xo_pybind11_dependency(${SELF_LIB} xo_arena)

# end CMakeLists.txt
