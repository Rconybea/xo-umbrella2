# xo_pyunit/src/pyunit/CMakeLists.txt

set(SELF_LIB pyunit)
set(SELF_SRCS pyunit.cpp)

# ----------------------------------------------------------------

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} xo_pyutil)
xo_pybind11_dependency(${SELF_LIB} xo_unit)
xo_pybind11_dependency(${SELF_LIB} refcnt)
xo_pybind11_dependency(${SELF_LIB} indentlog)

# CMakeLists.txt
