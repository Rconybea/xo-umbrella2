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
    /** default logbuf arena size, when the caller's PpConfig does not set one.
     *
     *  ArenaConfig::size_ defaults to 0, and a PrettySink given a zero-sized
     *  logbuf aborts -- so every one of the 23 hand-rolled copies had to supply
     *  a size, and every one of them chose 64k.  That makes it this function's
     *  default rather than each caller's decision.
     **/
    static constexpr std::size_t c_toppstr_default_logbuf_z = 64 * 1024;

    namespace detail {
        /** fresh logbuf ArenaConfig, with a name unique to this call.
         *
         *  The uniqueness is NOT optional and is why callers should not build
         *  their own: two PrettySinks sharing an ArenaConfig name interfere,
         *  and the symptom is wrong indentation in whichever renders second --
         *  a silent wrong answer, not an error.  Six separate files carried a
         *  comment warning about this before it was encapsulated here.
         *
         *  @p size  arena size, taken from the caller's config so that raising
         *           it for a large rendering still works.
         **/
        inline xo::mm::ArenaConfig
        toppstr_logbuf_config(std::size_t size) {
            static std::atomic<int> s_seq{0};

            return xo::mm::ArenaConfig{
                .name_ = "xo.toppstr." + std::to_string(s_seq.fetch_add(1) + 1),
                .size_ = (size ? size : c_toppstr_default_logbuf_z) };
        }
    } /*namespace detail*/

    /** render @p args (concatenated, no separator) through a PrettySink
     *  configured by @p cfg, and return the result.
     *
     *  @p cfg supplies the margins and arena size; its logbuf NAME is replaced,
     *  see detail::toppstr_logbuf_config.
     **/
    template <typename... Ts>
    std::string
    toppstr(const PpConfig & cfg, const Ts &... args) {
        PrettySink pps(PpConfig(cfg)
                       .with_logbuf_config(
                                           detail::toppstr_logbuf_config(cfg.logbuf().logbuf_config().size_)),
                       nullptr /*out*/);

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
        return toppstr(PpConfig(), a0, args...);
    }
} /*namespace xo::pp*/

/* end toppstr.hpp */
