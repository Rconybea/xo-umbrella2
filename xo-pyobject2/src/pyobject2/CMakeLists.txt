# xo_pyobject2/src/pyobject2/CMakeLists.txt

set(SELF_LIB xo_pyobject2)
set(SELF_SRCS pyobject2.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
# pyutil.hpp includes <xo/refcnt/Refcounted.hpp>; xo_pyutil is an INTERFACE
# target so its own deps do not reach us (cf. xo-pyarena, same pattern)
xo_pybind11_dependency(${SELF_LIB} refcnt)
xo_pybind11_dependency(${SELF_LIB} xo_object2)
# DObjectHandle + AllocFlywheel (the proxy and what it pins a root in)
xo_pybind11_dependency(${SELF_LIB} xo_facet)
# APrintable: the facet DFloat's handle is keyed on
xo_pybind11_dependency(${SELF_LIB} xo_printable2)
# IAllocator_DArena: obj<AAllocator> over the flywheel's arena
xo_pybind11_dependency(${SELF_LIB} xo_alloc2)
# PpSink: Float.pretty's argument type
xo_pybind11_dependency(${SELF_LIB} xo_ppsink)
# AllocFlywheel / PpSink are REGISTERED by these, which this module imports at
# init.  header_dependency: we need their generated .hpp, not their libs
xo_pybind11_header_dependency(${SELF_LIB} xo_pyfacet)
xo_pybind11_header_dependency(${SELF_LIB} xo_pyindentlog2)

# end CMakeLists.txt
