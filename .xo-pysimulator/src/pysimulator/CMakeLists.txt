# xo_pysimulator/src/pysimulator/CMakeLists.txt

set(SELF_LIB xo_pysimulator)
set(SELF_SRCS pysimulator.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_header_dependency(${SELF_LIB} xo_pyreactor)
xo_pybind11_dependency(${SELF_LIB} simulator)
xo_pybind11_header_dependency(${SELF_LIB} xo_pyutil)
