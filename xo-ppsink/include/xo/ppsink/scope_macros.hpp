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

/** capture a scope_setup at level @p lvl, but only *enabled* when @p flag is
 *  true (otherwise pinned to log_level::never, i.e. disabled).  Mirrors legacy
 *  XO_ENTER1: the second arg is a runtime enable flag, NOT a banner argument.
 **/
#define XO_ENTER1_(lvl, flag) \
    xo::pp::scope_setup{ __PRETTY_FUNCTION__, \
                         ((flag) ? xo::pp::log_level::lvl \
                                 : xo::pp::log_level::never), \
                         xo::pp::scope_config::function_style, \
                         __FILE__, \
                         __LINE__ }

/** capture a scope_setup enabled iff @p flag is true (at log_level::always).
 *  Use as: xo::pp::scope log(XO_DEBUG_(some_flag));  -- logs only when the
 *  flag holds.  Mirrors legacy XO_DEBUG = XO_ENTER1(always, flag).
 **/
#define XO_DEBUG_(flag) XO_ENTER1_(always, flag)

#endif /* XO_PPSINK_SCOPE_MACROS_HPP */

/* end scope_macros.hpp */
