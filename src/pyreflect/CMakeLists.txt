# xo_pyreflect/CMakeLists.txt

set(SELF_LIB pyreflect)
set(SELF_SRCS pyreflect.cpp)

# ----------------------------------------------------------------
# pybind11 dep

xo_pybind11_library(${SELF_LIB} ${SELF_SRCS})

xo_dependency(${SELF_LIB} reflect)
