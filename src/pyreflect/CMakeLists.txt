# xo_pyreflect/src/pyreflect/CMakeLists.txt

set(SELF_LIB xo_pyreflect)
set(SELF_SRCS pyreflect.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} reflect)
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
