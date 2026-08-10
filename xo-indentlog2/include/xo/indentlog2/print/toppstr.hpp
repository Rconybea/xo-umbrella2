/** @file toppstr.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  xo::pp::toppstr(args...): build a std::string by rendering @p args, in
 *  order, through a PrettySink -- i.e. WITH line breaking.
 *
 *  The counterpart to xo::pp::tostr (xo/ppsink/tostr.hpp), which renders
 *  through a FlatSink and deliberately never breaks a line.  Same relationship
 *  as legacy xo::toppstr2(ppconfig, args...)
 *  (xo/indentlog/print/ppstr.hpp) had to legacy xo::tostr.
 *
 *  This cannot live beside tostr in xo-ppsink: PrettySink is xo-indentlog2's,
 *  and xo-ppsink sits below it in the level order.
 *
 *  Before this existed, 23 files built a PrettySink by hand to do this -- three
 *  of them production code, not tests.  See
 *  .xo-backlog/xo-indentlog2/issues/01-toppstr-prettysink-to-string.md
 **/

#pragma once

#include "PrettySink.hpp"
#include "PpConfig.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <atomic>
#include <concepts>
#include <cstdint>
#include <string>
#include <type_traits>

namespace xo::pp {
    /** render @p args (concatenated, no separator) through a PrettySink
     *  configured by @p cfg, and return the result.
     *
     *  @p cfg supplies the margins and arena size; its logbuf NAME is replaced,
     *  see detail::toppstr_logbuf_config.
     *
     *  Caller may use PpConfig::plain() / PpConfig::colored() for example
     **/
    template <typename... Ts>
    std::string
    toppstr(const PpConfig & cfg, const Ts &... args) {
        PrettySink pps(cfg, nullptr /*out*/);

        (pps.pp(args), ...);

        return std::string(pps.output());
    }

    /** render @p args through a PrettySink with default configuration.
     *
     *  Constrained so a leading PpConfig picks the overload above rather than
     *  being rendered as a value.
     **/
    template <typename T0, typename... Ts>
        requires (!std::same_as<std::remove_cvref_t<T0>, PpConfig>)
    std::string
    toppstr(const T0 & a0, const Ts &... args) {
        return toppstr(PpConfig::colored(), a0, args...);
    }
} /*namespace xo::pp*/

/* end toppstr.hpp */
