cmake_minimum_required(VERSION 3.10)

project(indentlog VERSION 0.1)
enable_language(CXX)

include(cmake/nestlog.cmake)
include(cmake/code-coverage.cmake)

enable_testing()
# activate code coverage for all executables + libraries (when -DCODE_COVERAGE=ON)
add_code_coverage()

# 1. assuming that /nix/store/ prefixes .hpp files belonging to gcc, catch2 etc.
#    we're not interested in code coverage for these sources.
# 2. exclude the utest/ subdir, we don't need coverage on the unit tests themselves;
#    rather,  want coverage on the code that the unit tests exercise.
#
add_code_coverage_all_targets(EXCLUDE /nix/store/* utest/*)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# always write compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(NOT USER)
    set(USER $ENV{USER})
endif()

# hmm. this works if explicitly given with cmake:
#    cmake -DCMAKE_INSTALL_PREFIX=/home/roland/local path/to/source
# but not as default
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX /home/${USER}/local CACHE STRING "install directory")
endif()
if(NOT CMAKE_INSTALL_RPATH)
    set(CMAKE_INSTALL_RPATH /home/${USER}/local/lib CACHE STRING "runpath in installed libraries/executables")
endif()

add_subdirectory(example)
add_subdirectory(utest)

# header-only library.
# see [[https://stackoverflow.com/questions/47718485/install-and-export-interface-only-library-cmake]]
#
add_library(indentlog INTERFACE)
target_include_directories(indentlog INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/indentlogConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

install(
    TARGETS indentlog
    EXPORT indentlogTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
    )

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/indentlogConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/indentlogConfig.cmake"
    INSTALL_DESTINATION lib/cmake/indentlog
    )

install(EXPORT indentlogTargets DESTINATION lib/cmake/indentlog)
install(
    FILES
    "${PROJECT_BINARY_DIR}/indentlogConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/indentlogConfig.cmake"
    DESTINATION lib/cmake/indentlog)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

install(TARGETS hello DESTINATION bin/indentlog/example)
install(TARGETS ex1 DESTINATION bin/indentlog/example)
install(TARGETS ex2 DESTINATION bin/indentlog/example)
install(TARGETS ex3 DESTINATION bin/indentlog/example)
install(TARGETS ex4 DESTINATION bin/indentlog/example)
