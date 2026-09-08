# xo_pyreactor2/src/pyreactor2/CMakeLists.txt

set(SELF_LIB xo_pyreactor2)
set(SELF_SRCS pyreactor2.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} xo_reactor2)
