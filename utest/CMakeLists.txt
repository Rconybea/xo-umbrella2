# xo-kalmanfilter/utest/CMakeLists.txt

set(SELF_EXE utest.filter)

set(SELF_SRCS
    KalmanFilter.test.cpp
    filter_utest_main.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
target_code_coverage(${SELF_EXE} AUTO ALL)

# ----------------------------------------------------------------
# create convenience symlink to canned data

#create_symlink("${CMAKE_SOURCE_DIR}/utestdata/" "src/filter/utest/utestdata")

# ----------------------------------------------------------------
# generic project dependency

# PROJECT_SOURCE_DIR:
#   so we can for example write
#     #include "logutil/scope.hpp"
#   from anywhere in the project
# PROJECT_BINARY_DIR:
#   since version file will be in build directory,  need that directory
#   to also be included in compiler's include path
#
xo_self_dependency(${SELF_EXE} xo_kalmanfilter)

# ----------------------------------------------------------------
# internal dependencies: logutil, ...

xo_dependency(${SELF_EXE} xo_statistics)

# ----------------------------------------------------------------
# external dependencies: catch2..

xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# ----------------------------------------------------------------
# make standard directories for std:: includes explicit
# so that
# (1) they appear in compile_commands.json.
# (2) clangd (run from emacs lsp-mode) can find them
#
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# end CMakeLists.txt
