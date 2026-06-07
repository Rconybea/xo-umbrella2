# xo-ratio/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_ratio VERSION 1.0)
enable_language(CXX)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

# one-time project-specific c++ flags.  usually empty
set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

add_subdirectory(example)
add_subdirectory(utest)

# ----------------------------------------------------------------
# provide find_package() support for projects using this library

set(SELF_LIB xo_ratio)
xo_add_headeronly_library(${SELF_LIB})
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

xo_headeronly_dependency(${SELF_LIB} xo_reflectutil)
xo_headeronly_dependency(${SELF_LIB} xo_flatstring)

if (XO_ENABLE_EXAMPLES)
    install(TARGETS ratio_ex1 DESTINATION bin/ratio/example)
endif()

# ----------------------------------------------------------------
# docs targets depend on all the other library/utest targets,
# must come after them
#
add_subdirectory(docs)

# end CMakeLists.txt
