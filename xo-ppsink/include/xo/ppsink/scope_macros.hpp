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

# include "scope.hpp"

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

/** capture a scope_setup at level @p lvl, enabled only when @p flag is true,
 *  banner-named @p name1 instead of the enclosing function.  Mirrors legacy
 *  XO_ENTER2.
 *
 *  Note @p name1 is passed through as-is: scope_config::function_style is
 *  deliberately NOT applied, since an explicit name is not a function
 *  signature and there is nothing to streamline.
 **/
#define XO_ENTER2_(lvl, flag, name1) \
    xo::pp::scope_setup{ (name1), \
                         ((flag) ? xo::pp::log_level::lvl \
                                 : xo::pp::log_level::never), \
                         xo::FunctionStyle::literal, \
                         __FILE__, \
                         __LINE__ }

/** capture a scope_setup enabled iff @p flag is true (at log_level::always),
 *  banner-named @p name1.  Mirrors legacy XO_DEBUG2.
 **/
#define XO_DEBUG2_(flag, name1) XO_ENTER2_(always, flag, name1)

/** capture a scope_setup at level @p lvl, bannered from TWO runtime names.
 *  Mirrors legacy XO_LITERAL.
 *
 *  @p name1 is printed styled/colored; @p name2 is appended verbatim with no
 *  separator, so the caller supplies its own -- e.g.
 *  XO_LITERAL_(verbose, self_type, "::ctor") banners as "Foo::ctor".
 *
 *  Both names are runtime string_views, so this is the form to reach for when
 *  the banner is assembled from values rather than __PRETTY_FUNCTION__.
 **/
#define XO_LITERAL_(lvl, name1, name2) \
    xo::pp::scope_setup{ (name1), \
                         xo::pp::log_level::lvl, \
                         xo::FunctionStyle::literal, \
                         __FILE__, \
                         __LINE__, \
                         (name2) }

#endif /* XO_PPSINK_SCOPE_MACROS_HPP */

/* end scope_macros.hpp */
