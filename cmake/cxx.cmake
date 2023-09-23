# ----------------------------------------------------------------
# use this in subdirs that compile c++ code
#
macro(xo_include_options target)
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
      ${PROJECT_SOURCE_DIR}/include
      ${PROJECT_BINARY_DIR}
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
# use this when relying on indentlog [[https://github.com/rconybea/indentlog]] headers
#
macro(xo_indentlog_dependency target)
    find_package(indentlog REQUIRED)
    #add_dependencies(${target} indentlog)
    target_link_libraries(${target} PUBLIC indentlog)
    #target_include_directories(${target} PUBLIC ${indentlog_DIR}/../../../include)
endmacro()
