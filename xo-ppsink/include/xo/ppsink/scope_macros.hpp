/** @file scope_macros.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Opt-in convenience macros for xo::pp::scope.
 *
 *  Named with a trailing underscore (XO_ENTER0_ / XO_SCOPE_) so they do NOT
 *  collide with the legacy <xo/indentlog/scope.hpp> macros (XO_ENTER0 /
 *  XO_SCOPE).  That lets a translation unit use these even when it also pulls
 *  in legacy xo-indentlog -- e.g. transitively via xo-arena / PrettySink.
 *  (Temporary: once legacy xo-indentlog is retired, drop the trailing
 *  underscore.)
 *
 *  Kept out of scope.hpp so scope.hpp itself stays macro-free.
 **/

#ifndef XO_PPSINK_SCOPE_MACROS_HPP
#define XO_PPSINK_SCOPE_MACROS_HPP

#include "scope.hpp"

/** capture a scope_setup for the enclosing function, at log level @p lvl.
 *  Uses __PRETTY_FUNCTION__ + the configured scope_config::function_style, so
 *  a class method banners as "Class::method" (streamlined, the default).
 **/
#define XO_ENTER0_(lvl) \
    xo::pp::scope_setup{ __PRETTY_FUNCTION__, \
                         xo::pp::log_level::lvl, \
                         xo::pp::scope_config::function_style, \
                         __FILE__, \
                         __LINE__ }

/** declare an RAII scope logger @p varname for the enclosing function **/
#define XO_SCOPE_(varname, lvl) xo::pp::scope varname(XO_ENTER0_(lvl))

#endif /* XO_PPSINK_SCOPE_MACROS_HPP */

/* end scope_macros.hpp */
