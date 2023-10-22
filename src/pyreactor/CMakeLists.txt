# xo_pyreactor/src/pyreactor/CMakeLists.txt

set(SELF_LIB pyreactor)
set(SELF_SRCS pyreactor.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})

xo_pybind11_dependency(${SELF_LIB} reactor)
xo_pybind11_header_dependency(${SELF_LIB} pyprintjson)
