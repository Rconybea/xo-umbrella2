# xo-pyexpression/src/pyexpression/CMakeLists.txt

set(SELF_LIB xo_pyexpression)
set(SELF_SRCS pyexpression.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} xo_expression)
xo_pybind11_dependency(${SELF_LIB} xo_pyreflect)
xo_dependency(${SELF_LIB} refcnt)
