# xo-pyjit/src/pyjit/CMakeLists.txt

set(SELF_LIB xo_pyjit)
set(SELF_SRCS pyjit.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
xo_pybind11_dependency(${SELF_LIB} xo_jit)
xo_pybind11_dependency(${SELF_LIB} xo_pyexpression)
xo_dependency(${SELF_LIB} refcnt)
