# xo-webutil/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(webutil VERSION 1.0)
enable_language(CXX)

# common XO cmake macros (see proj/xo-cmake)
include(xo_macros/xo_cxx)
include(xo_macros/code-coverage)

# ----------------------------------------------------------------
# unit test setup (no unit tests yet,  but want 'make tests' to do something)

enable_testing()
# activate code coverage for all executables + libraries (when -DCODE_COVERAGE=ON)
add_code_coverage()

# 1. assuming that /nix/store/ prefixes .hpp files belonging to gcc, catch2 etc.
#    we're not interested in code coverage for these sources.
# 2. exclude the utest/ subdir, we don't need coverage on the unit tests themselves;
#    rather,  want coverage on the code that the unit tests exercise.
#
add_code_coverage_all_targets(EXCLUDE /nix/store/* utest/*)

# ----------------------------------------------------------------

xo_toplevel_compile_options()

# ----------------------------------------------------------------

# header-only library.
# see [[https://stackoverflow.com/questions/47718485/install-and-export-interface-only-library-cmake]]
#
xo_add_headeronly_library(webutil)

# ----------------------------------------------------------------
# standard install

xo_install_library3(webutil ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# provide find_package() support

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# install bespoke targets, if any

#install(TARGETS example DESTINATION bin/xo-webutil/example)
