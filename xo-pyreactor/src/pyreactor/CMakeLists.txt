# xo_pyreactor/src/pyreactor/CMakeLists.txt

set(SELF_LIB xo_pyreactor)
set(SELF_SRCS pyreactor.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} reactor)
# reaches legacy xo-indentlog headers via xo-refcnt's compat header.
xo_pybind11_dependency(${SELF_LIB} indentlog)
xo_pybind11_header_dependency(${SELF_LIB} xo_pyprintjson)
