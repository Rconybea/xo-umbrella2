# xo-indentlog2/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(indentlog2 VERSION 1.0)
enable_language(CXX)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

# one-time project-specific c++ flags.  usually empty
#set(PROJECT_CXX_FLAGS "-Wstringop-overread")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

#add_subdirectory(example)
#add_subdirectory(utest)

# ----------------------------------------------------------------
# cmake export

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}TArgets)

# ----------------------------------------------------------------
# docs

# docs targets depend on all the other library/utest targets
#add_subdirectory(docs)

# end CMakeLists.txt
