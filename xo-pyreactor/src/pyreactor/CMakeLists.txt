# xo_pyreactor/src/pyreactor/CMakeLists.txt

set(SELF_LIB xo_pyreactor)
set(SELF_SRCS pyreactor.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} reactor)
# pyreactor.cpp includes <xo/ppsink/tostr.hpp> + <xo/ppsink/pp_time.hpp>.
xo_pybind11_dependency(${SELF_LIB} xo_ppsink)
xo_pybind11_header_dependency(${SELF_LIB} xo_pyprintjson)
