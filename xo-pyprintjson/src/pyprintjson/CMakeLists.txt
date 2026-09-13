# xo_pyprintjson/src/pyprintjson/CMakeLists.txt

set(SELF_LIB xo_pyprintjson)
set(SELF_SRCS pyprintjson.cpp)

# ----------------------------------------------------------------

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} printjson)
# ReflectAppcx is named directly by configure(); it is registered by
# xo_pyreflect, which this module imports at init
xo_pybind11_header_dependency(${SELF_LIB} xo_pyreflect)
xo_pybind11_dependency(${SELF_LIB} reflect)
# ReflectAppcx.hpp names Indentlog2Appcx, so that type must be complete here
xo_pybind11_dependency(${SELF_LIB} xo_indentlog2)
# MemorySizeInfo, the element type collect_pools() returns, is registered by
# xo_pyarena; CollectPools.hpp comes from the same subsystem
xo_pybind11_header_dependency(${SELF_LIB} xo_pyarena)
xo_pybind11_dependency(${SELF_LIB} xo_arena)
