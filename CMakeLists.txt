# xo-ratio/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_ratio VERSION 1.0)
enable_language(CXX)

# common XO cmake macros (see proj/xo-cmake)
include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

# ----------------------------------------------------------------
# unit test setup

enable_testing()

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=coverage

if (NOT DEFINED PROJECT_CXX_FLAGS_COVERAGE)
    # note: for clang would use -fprofile-instr-generate -fcoverage-mapping here instead and also at link time
    set(PROJECT_CXX_FLAGS_COVERAGE ${PROJECT_CXX_FLAGS} -ggdb -Og -fprofile-arcs -ftest-coverage
        CACHE STRING "coverage c++ compiler flags")
endif()
message("-- PROJECT_CXX_FLAGS_COVERAGE: coverage c++ flags are [${PROJECT_CXX_FLAGS_COVERAGE}]")

add_compile_options("$<$<CONFIG:COVERAGE>:${PROJECT_CXX_FLAGS_COVERAGE}>")
# when -DCMAKE_BUILD_TYPE=coverage, link executables with gcov
link_libraries("$<$<CONFIG:COVERAGE>:gcov>")

find_program(LCOV_EXECUTABLE NAMES lcov)
find_program(GENHTML_EXECUTABLE NAMES genhtml)

# with coverage build:
# 1. invoke instrumented executables for which you want coverage:
#     (cd path/to/build && ctest)
# 2. post-process low-level coverage data
#     (path/to/build/gen-ccov)
# 3. point browser to generated html data
#     file:///path/to/build/ccov/html/index.html
#
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/gen-ccov.in
    ${PROJECT_BINARY_DIR}/gen-ccov)

file(CHMOD ${PROJECT_BINARY_DIR}/gen-ccov PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# ----------------------------------------------------------------
# c++ settings

# one-time project-specific c++ flags.  usually empty
set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

xo_toplevel_compile_options()

# ----------------------------------------------------------------

add_subdirectory(example)
add_subdirectory(utest)
add_subdirectory(docs)

# ----------------------------------------------------------------
# provide find_package() support for projects using this library

set(SELF_LIB xo_ratio)
xo_add_headeronly_library(${SELF_LIB})
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# dependencies

xo_headeronly_dependency(${SELF_LIB} xo_flatstring)
#xo_headeronly_dependency(${SELF_LIB} randomgen)
# etc..

# end CMakeLists.txt
