# xo-ppsink/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_ppsink VERSION 1.0)
enable_language(CXX)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

# one-time project-specific c++ flags. usually empty
set(PROJECT_CXX_FLAGS "")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------
# output targets

add_subdirectory(src/ppsink)
add_subdirectory(example)
add_subdirectory(utest)

# ----------------------------------------------------------------
# cmake export

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------

if (XO_ENABLE_EXAMPLES)
    install(TARGETS ppsink_ex1 DESTINATION bin/ppsink/example)
    install(TARGETS ppsink_ex2 DESTINATION bin/ppsink/example)
    install(TARGETS ppsink_ex3a DESTINATION bin/ppsink/example)
    install(TARGETS ppsink_ex3b DESTINATION bin/ppsink/example)
endif()

# end CMakeLists.txt
