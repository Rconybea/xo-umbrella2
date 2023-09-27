
set(XO_STANDARD_COMPILE_OPTIONS -Werror -Wall -Wextra)
set(XO_COMPILE_OPTIONS ${XO_STANDARD_COMPILE_OPTIONS})

# ----------------------------------------------------------------
# use this in subdirs that compile c++ code
#
macro(xo_include_options2 target)
    # ----------------------------------------------------------------
    # PROJECT_SOURCE_DIR:
    #   so we can for example write
    #     #include "ordinaltree/foo.hpp"
    #   from anywhere in the project
    # PROJECT_BINARY_DIR:
    #   since generated version file will be in build directory,
    #   need that build directory to also appear in
    #   compiler's include path
    #
    target_include_directories(
      ${target} PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>              # e.g. for #include "indentlog/scope.hpp"
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${target}>    # e.g. for #include "Refcounted.hpp" in refcnt/src
      $<INSTALL_INTERFACE:include/${target}>
      $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>                      # e.g. for generated .hpp files
    )

    # ----------------------------------------------------------------
    # make standard directories for std:: includes explicit
    # so that
    # (1) they appear in compile_commands.json.
    # (2) clangd (run from emacs lsp-mode) can find them
    #
    if(CMAKE_EXPORT_COMPILE_COMMANDS)
      set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    endif()
endmacro()

# ----------------------------------------------------------------
#
macro(xo_compile_options target)
    target_copmile_options(${target} PRIVATE ${XO_COMPILE_OPTIONS})
endmacro()

# ----------------------------------------------------------------
#
# dependency on an xo library (including header-only libraries)
# e.g. indentlog
#
# An xo package foo works with cmake
#   find_package(foo)
# by providing plugin .cmake files in
#   ${PREFIX}/lib/cmake/foo/fooConfig.cmake
#   ${PREFIX}/lib/cmake/foo/fooConfigVersion.cmake
#   ${PREFIX}/lib/cmake/foo/fooTargets.cmake
#
# dep: name of required dependency,  e.g. indentlog
#
macro(xo_internal_dependency target dep)
    find_package(${dep} CONFIG REQUIRED)
    target_link_libraries(${target} PUBLIC ${dep})
endmacro()
