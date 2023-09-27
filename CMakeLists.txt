cmake_minimum_required(VERSION 3.10)

project(subsys VERSION 0.1)
enable_language(CXX)

include (xo_macros/xo_cxx)
include(cmake/cxx.cmake)
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

set(XO_PROJECT_NAME subsys)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED True)

# always write compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# ----------------------------------------------------------------
# - author's convenience:  default install prefix to /home/$USER/local
# - otherwise use -DCMAKE_INSTALL_PREFIX=/path/to/somewhere

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
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib CACHE STRING "runpath in installed libraries/executables")
endif()

#add_subdirectory(example)
#add_subdirectory(utest)

# ----------------------------------------------------------------
# installing header-only library

add_library(subsys INTERFACE)
target_include_directories(subsys INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
xo_install_library(subsys)

# ----------------------------------------------------------------
# cmake export
# (so this library works with cmake's find_package())

set(XO_PROJECT_CONFIG_VERSION "${XO_PROJECT_NAME}ConfigVersion.cmake")
set(XO_PROJECT_CONFIG "${XO_PROJECT_NAME}Config.cmake")

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/${XO_PROJECT_CONFIG_VERSION}"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${XO_PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${XO_PROJECT_CONFIG}"
    INSTALL_DESTINATION lib/cmake/${XO_PROJECT_NAME}
    )

install(EXPORT ${XO_PROJECT_NAME}Targets DESTINATION lib/cmake/${XO_PROJECT_NAME})
install(
    FILES
    "${PROJECT_BINARY_DIR}/${XO_PROJECT_CONFIG_VERSION}"
    "${PROJECT_BINARY_DIR}/${XO_PROJECT_CONFIG}"
    DESTINATION lib/cmake/${XO_PROJECT_NAME})

# ----------------------------------------------------------------
# install .hpp

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

# end CMakeLists.txt
