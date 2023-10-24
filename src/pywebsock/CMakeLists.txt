# xo_pywebsock/src/pywebsock/CMakeLists.txt

set(SELF_LIB pywebsock)
set(SELF_SRCS pywebsock.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} websock)
xo_pybind11_header_dependency(${SELF_LIB} pywebutil)
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
