/* @file Refcounted_indentlog.hpp
 *
 * Transitional compatibility header: Refcounted.hpp plus the legacy
 * xo-indentlog scope logger (scope/xtag/tostr, and timeutil through it).
 *
 * Refcounted.hpp itself is now ppsink-only.  Code not yet migrated to
 * xo-ppsink that used to get legacy scope logging transitively through
 * Refcounted.hpp should include this header instead (or include
 * <xo/indentlog/scope.hpp> explicitly).  Retire this once the consuming
 * subsystems (reader, reader2, interpreter2, reactor, process, ...) are
 * migrated off legacy indentlog.
 *
 * NB: xo-refcnt no longer declares an xo-indentlog dependency, so a consumer
 * that includes this header must declare one itself.  (refcnt used to
 * propagate indentlog PUBLICly, which put it on every consumer's command line
 * and hid whether downstream code had actually migrated.)
 */

#pragma once

#include "Refcounted.hpp"
#include <xo/indentlog/scope.hpp>

/* end Refcounted_indentlog.hpp */
