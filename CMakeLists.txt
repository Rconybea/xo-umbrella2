# reflect/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

message(CMAKE_VERSION=${CMAKE_VERSION})

project(reflect VERSION 0.1)
enable_language(CXX)

# common XO cmake macros (see proj/xo-cmake)
include(xo_macros/xo_cxx)
include(cmake/cxx.cmake)
include(cmake/code-coverage.cmake)

# ----------------------------------------------------------------
# unit test setup

enable_testing()
# activate code coverage for all executables + libraries (when configured with -DCODE_COVERAGE=ON)
add_code_coverage()
# 1. assuming that /nix/store/ prefixes .hpp files belonging to gcc, catch2 etc.
#    we're not interested in code coverage for these sources.
# 2. exclude the utest/ subdir, we don't need coverage on the unit tests themselves;
#    rather,  want coverage on the code that the unit tests exercise.
#
# NOTE: this seems to work only with the 'ccov-all' target.  In particular,  doesn't seem to do anything with the 'ccov' target
#
add_code_coverage_all_targets(EXCLUDE /nix/store/* ${PROJECT_SOURCE_DIR}/utest/* ${PROJECT_BINARY_DIR}/local/* ${PROJECT_SOURCE_DIR}/repo/*)

# ----------------------------------------------------------------
# c++ settings

set(XO_PROJECT_NAME reflect)
set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")

add_definitions(${PROJECT_CXX_FLAGS})

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED True)

# always write compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(NOT CMAKE_INSTALL_RPATH)
    set(CMAKE_INSTALL_RPATH $(CMAKE_INSTALL_PREFIX)/lib CACHE STRING "runpath in installed libraries/executables")
endif()

# early find_package() experiment
find_package(indentlog CONFIG REQUIRED)
find_package(refcnt CONFIG REQUIRED)

# ----------------------------------------------------------------
# sources

add_subdirectory(src/reflect)
add_subdirectory(utest)

# ----------------------------------------------------------------
# cmake export:
#
# populate .cmake files in $CMAKE_INSTALL_LIBDIR/cmake/reflect.
# cmake projects that include this directory in $CMAKE_PREFIX_PATH
# can use
#   find_package(reflect REQUIRED)
# and
#   target_link_libraries(${sometarget} PUBLIC reflect)
# to use the reflect library

set(XO_PROJECT_CONFIG_VERSION "${XO_PROJECT_NAME}ConfigVersion.cmake")
set(XO_PROJECT_CONFIG "${XO_PROJECT_NAME}Config.cmake")

include(CMakePackageConfigHelpers)

# generates build/reflectConfigVersion.cmake
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${XO_PROJECT_CONFIG_VERSION}"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

# generates build/reflectConfig.cmake
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${XO_PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${XO_PROJECT_CONFIG}"
    INSTALL_DESTINATION lib/cmake/${XO_PROJECT_NAME}
)

# creates {reflectTargets.cmake, reflectTargets-noconfig.cmake} in $CMAKE_INSTALL_LIBDIR/cmake/reflect/
# requires
#   install(.. EXPORT reflectTargets ..)
#
install(
    EXPORT ${XO_PROJECT_NAME}Targets
    DESTINATION lib/cmake/${XO_PROJECT_NAME}
)

# creates {reflectConfigVersion.cmake, reflectConfig.cmake} in $CMAKE_INSTALL_LIBDIR/cmake/reflect/
install(
    FILES
    "${PROJECT_BINARY_DIR}/${XO_PROJECT_CONFIG_VERSION}"
    "${PROJECT_BINARY_DIR}/${XO_PROJECT_CONFIG}"
    DESTINATION lib/cmake/${XO_PROJECT_NAME})

# ----------------------------------------------------------------
# install .hpp files

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/reflect/ DESTINATION include/reflect)
