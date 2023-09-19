cmake_minimum_required(VERSION 3.10)

project(nestlog VERSION 0.1)
enable_language(CXX)
enable_testing()

# always write compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(CMAKE_INSTALL_PREFIX /home/roland/local)
set(CMAKE_INSTALL_RPATH /home/roland/local/lib)

include(cmake/nestlog.cmake)
add_subdirectory(example)

# header-only library
#add_library(indentlog INTERFACE)
#target_include_directories(indentlog INTERFACE
#  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include>
#  )
#
#install(TARGETS indentlog
#  PUBLIC_HEADER DESTINATION include)  # COMPONENT Development

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)
