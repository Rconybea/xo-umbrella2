# xo_pyreactor/src/pyreactor/CMakeLists.txt

set(SELF_LIB xo_pyreactor)
set(SELF_SRCS pyreactor.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} reactor)
# pyreactor.cpp includes <xo/indentlog2/print/tostr.hpp> + <xo/ppsink/pp_time.hpp>;
# indentlog2 brings ppsink with it.
xo_pybind11_dependency(${SELF_LIB} xo_indentlog2)
xo_pybind11_header_dependency(${SELF_LIB} xo_pyprintjson)
