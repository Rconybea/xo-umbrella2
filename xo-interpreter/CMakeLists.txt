# xo-interpreter/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project (xo_interpreter VERSION 0.1)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")  # gcc-only!
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

# note on ordering: must read .cmake defn of lib before configuring any examples
add_subdirectory(src/interpreter)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------

#add_subdirectory(example)
add_subdirectory(utest)

# ----------------------------------------------------------------

#if (XO_ENABLE_EXAMPLES)
#    install(TARGETS xo_interpreter_ex1 DESTINATION bin/xo/example)
#endif()

# ----------------------------------------------------------------
# docs targets depend on all other library/utest targets
#
#add_subdirectory(docs)

# end CMakeLists.txt
