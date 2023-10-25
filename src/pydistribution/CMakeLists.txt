# xo_pydistribution/src/pydistribution/CMakeLists.txt

set(SELF_LIB pydistribution)
set(SELF_SRCS pydistribution.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} xo_distribution)
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
