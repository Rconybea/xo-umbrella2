## ----------------------------------------------------------------
## variable
##   XO_ADDRESS_SANITIZE
## determines whether to enable address sanitizer for the XO project
## (see toplevel CMakeLists.txt)
## ----------------------------------------------------------------
#if(XO_ADDRESS_SANITIZE)
#    add_compile_options(-fsanitize=address)
#    add_link_options(-fsanitize=address)
#endif()
#
## XO_STANDARD_COMPILE_OPTIONS: use these when XO_ADDRESS_SANITIZE=OFF
#set(XO_STANDARD_COMPILE_OPTIONS -Werror -Wall -Wextra)
#
## XO_ADDRESS_SANITIZE_COMPILE_OPTIONS: use when XO_ADDRESS_SANITIZE=ON
##
## address sanitizer build complains about _FORTIFY_SOURCE redefines
##    In file included from <built-in>:460:
##    <command line>:1:9: error: '_FORTIFY_SOURCE' macro redefined [-Werror,-Wmacro-redefined]
##    #define _FORTIFY_SOURCE 2
##
#set(XO_ADDRESS_SANITIZE_COMPILE_OPTIONS -Werror -Wall -Wextra -Wno-macro-redefined)
#
## XO_COMPILE_OPTIONS: use these with xo_compile_options() macro
#if(XO_ADDRESS_SANITIZE)
#    set(XO_COMPILE_OPTIONS ${XO_ADDRESS_SANITIZE_COMPILE_OPTIONS})
#else()
#    set(XO_COMPILE_OPTIONS ${XO_STANDARD_COMPILE_OPTIONS})
#endif()
#
## ----------------------------------------------------------------
## generally want all the errors+warnings!
## however: address sanitizer generates error on _FORTIFY_SOURCE
##
#macro(xo_compile_options target)
#    target_compile_options(${target} PRIVATE ${XO_COMPILE_OPTIONS})
#endmacro()
