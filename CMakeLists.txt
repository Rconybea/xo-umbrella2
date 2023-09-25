# reflect/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(reflect VERSION 0.1)
enable_language(CXX)

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

# ----------------------------------------------------------------
# external projects (need these to exist before add_subdirectory() below)
#
# we are expecting these projects to coexist peacefully in build/local
# (i.e. can run their `make install` steps independently with prefix build/local,
#       without any collisions)
#

include(ExternalProject)

## ----- indentlog ------

# NOTE:  we could have cmake handle git interaction,
#        but we want source for certain dependencies to live in a location
#        that's suitable for accepting changes + coordinated commits.
#        In particular,  not in the build directory!
#
externalproject_add(
    project_indentlog
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/repo/indentlog
    BINARY_DIR ${PROJECT_BINARY_DIR}/ext/indentlog
    INSTALL_DIR ${PROJECT_BINARY_DIR}/local
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCODE_COVERAGE=${CODE_COVERAGE} -DCMAKE_PREFIX_PATH=<INSTALL_DIR> -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    BUILD_COMMAND make
    INSTALL_COMMAND make install
    TEST_BEFORE_INSTALL True
)

add_library(indentlog INTERFACE IMPORTED)
#set_property(TARGET indentlog PROPERTY IMPORTED_LOCATION ${PROJECT_BINARY_DIR}/local/lib/libindentlog.so)
add_dependencies(indentlog project_indentlog)

# runs ctest in indentlog build dir
add_test(NAME indentlog COMMAND ${PROJECT_SOURCE_DIR}/cmake/run-external-ctest ${PROJECT_BINARY_DIR}/ext/indentlog)
#target_code_coverage(indentlog EXTERNAL AUTO ALL)

# ----- subsys -----

externalproject_add(
    project_subsys
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/repo/subsys
    BINARY_DIR ${PROJECT_BINARY_DIR}/ext/subsys
    INSTALL_DIR ${PROJECT_BINARY_DIR}/local
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCODE_COVERAGE=${CODE_COVERAGE} -DCMAKE_PREFIX_PATH=<INSTALL_DIR> -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    BUILD_COMMAND make
    INSTALL_COMMAND make install
    TEST_BEFORE_INSTALL True
)

add_library(subsys INTERFACE IMPORTED)
add_dependencies(subsys project_subsys)

# runs ctest in subsys build dir
add_test(NAME subsys COMMAND ${PROJECT_SOURCE_DIR}/cmake/run-external-ctest ${PROJECT_BINARY_DIR}/ext/subsys)

# ----- refcnt -----

# CMAKE_ARGS
#   CMAKE_BUILD_TYPE propagate Debug/Release build type
#   CODE_COVERAGE propagate code coverage setting
#   CMAKE_PREFIX_PATH path for support cmake files of dependencies (needed for find_package() to work)
#   CMAKE_INSTALL_PREFIX install subproject here
# SOURCE_DIR  -- where to find already established source code
# BINARY_DIR  -- run build for external project here
# INSTALL_DIR -- (temporarily) install external project here
#
externalproject_add(
    project_refcnt
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/repo/refcnt
    BINARY_DIR ${PROJECT_BINARY_DIR}/ext/refcnt
    INSTALL_DIR ${PROJECT_BINARY_DIR}/local
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCODE_COVERAGE=${CODE_COVERAGE} -DCMAKE_PREFIX_PATH=<INSTALL_DIR> -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    BUILD_COMMAND make
    INSTALL_COMMAND make install
    TEST_BEFORE_INSTALL True
)

add_library(refcnt SHARED IMPORTED)
set_property(TARGET refcnt PROPERTY IMPORTED_LOCATION ${PROJECT_BINARY_DIR}/local/lib/librefcnt.so)
add_dependencies(refcnt project_refcnt)
add_dependencies(refcnt project_indentlog)

# runs ctest in refcnt build dir
add_test(NAME refcnt COMMAND ${PROJECT_SOURCE_DIR}/cmake/run-external-ctest ${PROJECT_BINARY_DIR}/ext/refcnt)

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
