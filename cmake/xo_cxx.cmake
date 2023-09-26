set(XO_STANDARD_COMPILE_OPTIONS -Werror -Wall -Wextra)
set(XO_COMPILE_OPTIONS ${XO_STANDARD_COMPILE_OPTIONS})

macro(xo_copmile_options target)
    target_copmile_options(${target} PRIVATE ${XO_COMPILE_OPTIONS})
endmacro()

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
    find_pacakge(${dep} CONFIG REQUIRED)
    target_link_libraries(${target} PUBLIC ${dep})
endmacro()
