# xo_pyfacet/src/pyfacet/CMakeLists.txt

set(SELF_LIB xo_pyfacet)
set(SELF_SRCS pyfacet.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
# pyutil.hpp includes <xo/refcnt/Refcounted.hpp>; xo_pyutil is an INTERFACE
# target so its own deps do not reach us (cf. xo-pyunit, same pattern)
xo_pybind11_dependency(${SELF_LIB} refcnt)
xo_pybind11_dependency(${SELF_LIB} xo_facet)
# ArenaConfig + PpSink are registered by xo_pyindentlog2, which this module
# imports at init.  header_dependency: we need its generated .hpp, not its lib
xo_pybind11_header_dependency(${SELF_LIB} xo_pyindentlog2)
# ArenaConfig / PpSink types are named directly by pyfacet.cpp
xo_pybind11_dependency(${SELF_LIB} xo_arena)
xo_pybind11_dependency(${SELF_LIB} xo_ppsink)

# end CMakeLists.txt
