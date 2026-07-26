/** @file scope_macros.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Opt-in convenience macros for xo::print::scope.
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

/** capture a scope_setup for the enclosing function.
 *
 *  @p lvl (log level) is accepted for source-compatibility with the legacy
 *  xo-indentlog macro but ignored for now -- log-level gating is a later
 *  feature-parity step.
 **/
#define XO_ENTER0(lvl) xo::print::scope_setup{ __func__ }

/** declare an RAII scope logger @p varname for the enclosing function **/
#define XO_SCOPE(varname, lvl) xo::print::scope varname(XO_ENTER0(lvl))

#endif /* XO_PPSINK_SCOPE_MACROS_HPP */

/* end scope_macros.hpp */
