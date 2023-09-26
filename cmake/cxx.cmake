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
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>              # e.g. for #include "indentlog/scope.hpp"
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${target}>    # e.g. for #include "Refcounted.hpp" in refcnt/src
      $<INSTALL_INTERFACE:include/${target}>
      $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>                      # e.g. for generated config.hpp file
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
# variable
#   XO_ADDRESS_SANITIZE
# determines whether to enable address sanitizer for the XO project
# (see toplevel CMakeLists.txt)
# ----------------------------------------------------------------
if(XO_ADDRESS_SANITIZE)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

# XO_STANDARD_COMPILE_OPTIONS: use these when XO_ADDRESS_SANITIZE=OFF
set(XO_STANDARD_COMPILE_OPTIONS -Werror -Wall -Wextra)

# XO_ADDRESS_SANITIZE_COMPILE_OPTIONS: use when XO_ADDRESS_SANITIZE=ON
#
# address sanitizer build complains about _FORTIFY_SOURCE redefines
#    In file included from <built-in>:460:
#    <command line>:1:9: error: '_FORTIFY_SOURCE' macro redefined [-Werror,-Wmacro-redefined]
#    #define _FORTIFY_SOURCE 2
#
set(XO_ADDRESS_SANITIZE_COMPILE_OPTIONS -Werror -Wall -Wextra -Wno-macro-redefined)

# XO_COMPILE_OPTIONS: use these with xo_compile_options() macro
if(XO_ADDRESS_SANITIZE)
    set(XO_COMPILE_OPTIONS ${XO_ADDRESS_SANITIZE_COMPILE_OPTIONS})
else()
    set(XO_COMPILE_OPTIONS ${XO_STANDARD_COMPILE_OPTIONS})
endif()

# ----------------------------------------------------------------
# generally want all the errors+warnings!
# however: address sanitizer generates error on _FORTIFY_SOURCE
#
macro(xo_compile_options target)
    target_compile_options(${target} PRIVATE ${XO_COMPILE_OPTIONS})
endmacro()

# ----------------------------------------------------------------
# use this for a subdir that builds a library
# and supports find_package()
#
macro(xo_install_library target)
    install(
        TARGETS ${target}
        EXPORT ${target}Targets
        LIBRARY DESTINATION lib COMPONENT Runtime
        ARCHIVE DESTINATION lib COMPONENT Development
        RUNTIME DESTINATION bin COMPONENT Runtime
        PUBLIC_HEADER DESTINATION include COMPONENT Development
        BUNDLE DESTINATION bin COMPONENT Runtime
    )
endmacro()

# ----------------------------------------------------------------
# use this when relying on indentlog [[https://github.com/rconybea/indentlog]] headers
#
macro(xo_indentlog_dependency target)
    find_package(indentlog CONFIG REQUIRED)
    #add_dependencies(${target} indentlog)
    target_link_libraries(${target} PUBLIC indentlog)
    #target_include_directories(${target} PUBLIC ${indentlog_DIR}/../../../include)
endmacro()
