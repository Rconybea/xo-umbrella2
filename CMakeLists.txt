cmake_minimum_required(VERSION 3.10)

project(nestlog VERSION 0.1)
enable_language(CXX)

include(cmake/nestlog.cmake)
include(cmake/code-coverage.cmake)

enable_testing()
# activate code coverage for all executables + libraries (when -DCODE_COVERAGE=ON ??)
add_code_coverage()

# 1. assuming that /nix/store/ prefixes .hpp files belonging to gcc, catch2 etc.
#    we're not interested in code coverage for these sources.
# 2. exclude the utest/ subdir, we don't need coverage on the unit tests themselves;
#    rather,  want coverage on the code that the unit tests exercise.
#
add_code_coverage_all_targets(EXCLUDE /nix/store/* utest/*)

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

add_subdirectory(example)
add_subdirectory(utest)

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
