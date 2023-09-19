cmake_minimum_required(VERSION 3.10)

project(nestlog VERSION 0.1)
enable_language(CXX)
enable_testing()

# always write compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(NOT USER)
    set(USER $ENV{USER})
endif()

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX /home/${USER}/local CACHE STRING "install directory")
endif()
if(NOT CMAKE_INSTALL_RPATH)
    set(CMAKE_INSTALL_RPATH /home/${USER}/local/lib CACHE STRING "runpath in installed libraries/executables")
endif()

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

install(TARGETS hello DESTINATION bin/indentlog/example)
install(TARGETS ex1 DESTINATION bin/indentlog/example)
install(TARGETS ex2 DESTINATION bin/indentlog/example)
install(TARGETS ex3 DESTINATION bin/indentlog/example)
install(TARGETS ex4 DESTINATION bin/indentlog/example)
