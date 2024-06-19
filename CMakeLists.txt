# xo-pyreflect/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_pyreflect VERSION 0.1)
enable_language(CXX)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings (usually temporary)

set(PROJECT_CXX_FLAGS "")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------
# sources

# note: library=pyreflect (not xo_pyreflect) -> establishes pyreflectTargets
add_subdirectory(src/pyreflect)
#add_subdirectory(utest)

# ----------------------------------------------------------------
# provide find_package() support

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)
