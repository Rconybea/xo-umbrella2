cmake_minimum_required(VERSION 3.10)

project(indentlog VERSION 0.1)
enable_language(CXX)

# common XO cmake macros (see proj/xo-cmake)
include(xo_macros/xo_cxx)
include(xo_macros/code-coverage)

enable_testing()
# activate code coverage for all executables + libraries (when -DCODE_COVERAGE=ON)
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

# hmm. this works if explicitly given with cmake:
#    cmake -DCMAKE_INSTALL_PREFIX=/home/roland/local path/to/source
# but not as default
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX /home/${USER}/local CACHE STRING "install directory")
endif()

xo_toplevel_compile_options()

add_subdirectory(example)
add_subdirectory(utest)

# header-only library.
# see [[https://stackoverflow.com/questions/47718485/install-and-export-interface-only-library-cmake]]
#
add_library(indentlog INTERFACE)
xo_include_headeronly_options2(indentlog)
#target_include_directories(indentlog INTERFACE
#  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include>
#  )

# ----------------------------------------------------------------
# provide find_package() support

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------

xo_install_library2(${PROJECT_NAME})

# ----------------------------------------------------------------
# install .hpp

xo_install_include_tree()

# ----------------------------------------------------------------

install(TARGETS hello DESTINATION bin/indentlog/example)
install(TARGETS ex1 DESTINATION bin/indentlog/example)
install(TARGETS ex2 DESTINATION bin/indentlog/example)
install(TARGETS ex3 DESTINATION bin/indentlog/example)
install(TARGETS ex4 DESTINATION bin/indentlog/example)
