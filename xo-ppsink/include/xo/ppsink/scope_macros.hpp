/** @file scope_macros.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Opt-in convenience macros for xo::pp::scope.
 *
 *  These are deliberately kept OUT of scope.hpp: the legacy
 *  <xo/indentlog/scope.hpp> defines its own XO_ENTER0 / XO_SCOPE macros, and
 *  a translation unit that pulls in legacy xo-indentlog (e.g. transitively
 *  via xo-arena) alongside a macro-carrying scope.hpp would hit
 *  macro-redefinition conflicts.  By isolating the macros here, code that
 *  wants the terse spelling opts in explicitly -- and simply must not also
 *  include legacy xo-indentlog in the same TU.  Once legacy xo-indentlog is
 *  retired these can fold back into scope.hpp.
 **/

#ifndef XO_PPSINK_SCOPE_MACROS_HPP
#define XO_PPSINK_SCOPE_MACROS_HPP

#include "scope.hpp"

/** capture a scope_setup for the enclosing function, at log level @p lvl.
 *  Uses __PRETTY_FUNCTION__ + the configured scope_config::function_style, so
 *  a class method banners as "Class::method" (streamlined, the default).
 **/
#define XO_ENTER0(lvl) \
    xo::pp::scope_setup{ __PRETTY_FUNCTION__, \
                         xo::pp::log_level::lvl, \
                         xo::pp::scope_config::function_style }

/** declare an RAII scope logger @p varname for the enclosing function **/
#define XO_SCOPE(varname, lvl) xo::pp::scope varname(XO_ENTER0(lvl))

#endif /* XO_PPSINK_SCOPE_MACROS_HPP */

/* end scope_macros.hpp */
