# indentlog/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(indentlog VERSION 0.1)
enable_language(CXX)

# common XO cmake macros (see proj/xo-cmake)
include(xo_macros/xo_cxx)
include(xo_macros/code-coverage)

# ----------------------------------------------------------------
# unit test setup

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

add_subdirectory(example)
add_subdirectory(utest)

# header-only library.
# see [[https://stackoverflow.com/questions/47718485/install-and-export-interface-only-library-cmake]]
#
xo_add_headeronly_library(indentlog)

# ----------------------------------------------------------------
# standard install + provide find_package() support

#xo_install_library2(indentlog)
#xo_install_include_tree()
xo_install_library3(indentlog ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------

install(TARGETS hello DESTINATION bin/indentlog/example)
install(TARGETS ex1 DESTINATION bin/indentlog/example)
install(TARGETS ex2 DESTINATION bin/indentlog/example)
install(TARGETS ex3 DESTINATION bin/indentlog/example)
install(TARGETS ex4 DESTINATION bin/indentlog/example)
