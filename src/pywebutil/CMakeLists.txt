# xo-pywebutil/src/pywebutil/CMakeLists.txt

set(SELF_LIB pywebutil)
set(SELF_SRCS pywebutil.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} webutil)
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
