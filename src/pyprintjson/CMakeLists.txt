# xo_pyprintjson/src/pyprintjson/CMakeLists.txt

set(SELF_LIB xo_pyprintjson)
set(SELF_SRCS pyprintjson.cpp)

xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})

xo_pybind11_dependency(${SELF_LIB} printjson)
xo_pybind11_header_dependency(${SELF_LIB} xo_pyreflect)
