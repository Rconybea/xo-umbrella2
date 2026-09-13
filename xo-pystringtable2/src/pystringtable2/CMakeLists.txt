# xo_pystringtable2/src/pystringtable2/CMakeLists.txt

set(SELF_LIB xo_pystringtable2)
set(SELF_SRCS pystringtable2.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
# pyutil.hpp includes <xo/refcnt/Refcounted.hpp>; xo_pyutil is an INTERFACE
# target so its own deps do not reach us (cf. xo-pyfacet, same pattern)
xo_pybind11_dependency(${SELF_LIB} refcnt)
xo_pybind11_dependency(${SELF_LIB} xo_stringtable2)
# FacetAppcx is named directly by configure(); it is registered by xo_pyfacet,
# which this module imports at init.
# header_dependency: we need its generated .hpp, not its lib
xo_pybind11_header_dependency(${SELF_LIB} xo_pyfacet)
xo_pybind11_dependency(${SELF_LIB} xo_facet)
# MemorySizeInfo, the element type collect_pools() returns, is registered by
# xo_pyarena; CollectPools.hpp comes from the same subsystem
xo_pybind11_header_dependency(${SELF_LIB} xo_pyarena)
xo_pybind11_dependency(${SELF_LIB} xo_arena)

# end CMakeLists.txt
