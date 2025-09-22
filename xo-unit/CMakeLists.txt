# xo-unit/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_unit VERSION 1.0)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

# one-time project-specific c++ flags.  usually empty
set(PROJECT_CXX_FLAGS "-Wstringop-overread")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

add_subdirectory(example)
add_subdirectory(utest)

set(SELF_LIB xo_unit)
xo_add_headeronly_library(${SELF_LIB})
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)
xo_headeronly_dependency(${SELF_LIB} xo_flatstring)
xo_headeronly_dependency(${SELF_LIB} xo_ratio)

if (XO_ENABLE_EXAMPLES)
    install(TARGETS xo_unit_ex1    DESTINATION bin/xo/example)
    install(TARGETS xo_unit_ex2    DESTINATION bin/xo/example)
    install(TARGETS xo_unit_ex3    DESTINATION bin/xo/example)
    install(TARGETS xo_unit_ex4    DESTINATION bin/xo/example)
    install(TARGETS xo_unit_ex5    DESTINATION bin/xo/example)
    install(TARGETS xo_unit_ex6    DESTINATION bin/xo/example)
    install(TARGETS xo_unit_ex7    DESTINATION bin/xo/example)
    install(TARGETS xo_unit_ex8    DESTINATION bin/xo/example)
    install(TARGETS xo_unit_ex_qty DESTINATION bin/xo/example)
    install(TARGETS xo_unit_ex_su  DESTINATION bin/xo/example)
endif()

# ----------------------------------------------------------------
# docs targets depend on all the other library/utest targets,
# must come after them
#
add_subdirectory(docs)

# end CMakeLists.txt
