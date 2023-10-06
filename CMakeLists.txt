cmake_minimum_required(VERSION 3.10)

project(subsys VERSION 0.1)
enable_language(CXX)

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

set(XO_PROJECT_NAME subsys)

xo_toplevel_compile_options()

# ----------------------------------------------------------------
# installing header-only library

add_library(subsys INTERFACE)
xo_include_headeronly_options2(subsys)
xo_install_library2(subsys)

# ----------------------------------------------------------------
# provide find_package() support

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# install .hpp

xo_install_include_tree()

# end CMakeLists.txt
