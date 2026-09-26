# xo-pyreactor2websock/src/pyreactor2websock/CMakeLists.txt

set(SELF_LIB xo_pyreactor2websock)
set(SELF_SRCS pyreactor2websock.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} reactor2websock)
# XO_PYREACTOR_IMPORT_MODULE: AbstractSource, AbstractEventStore are bound there
xo_pybind11_header_dependency(${SELF_LIB} xo_pyreactor)
# XO_PYWEBUTIL_IMPORT_MODULE: StreamEndpointDescr, HttpEndpointDescr are bound there
xo_pybind11_header_dependency(${SELF_LIB} xo_pywebutil)
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
