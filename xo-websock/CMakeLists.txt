# xo-websock/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(websock VERSION 1.0)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

# one-time project-specific c++ flags.  usually empty
set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

add_subdirectory(src/websock)
# utest/ is NOT a unit test and is not merely disabled -- see
# .xo-backlog/xo-websock/issues/01.
#
# It is a manual browser demo: utest.websock serves http on port 7682 and you
# point a browser at localhost:7682/ex_websock.html (utest/README).  Its own
# CMakeLists leaves add_test() commented out for that reason, so enabling this
# line would register no ctest test even if it built.
#
# And it does not build: websock_utest_main.cpp is written against the
# pre-umbrella tree -- 25 unprefixed includes ("filter/...", "option/...",
# "indentlog/...").  17 of them are pure renames (filter/ -> xo/kalmanfilter/,
# time/Time.hpp -> xo/timeutil/timeutil.hpp, the rest just gaining the xo/
# prefix); the other 8 name two subsystems that do not exist in this tree at
# all, option (5) and volfit (3).
#
# It fails at the first compile, not at configure, because cmake turns the
# unknown `option'/`volfit' link targets into plain -l flags.
#
#add_subdirectory(utest)

# ----------------------------------------------------------------
# provide find_package() support for websock customers

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
