/** @file log_level.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Log-severity levels for the ppsink scope logger (POC subset ported from
 *  the legacy xo-indentlog log_level).  A scope is enabled when its level is
 *  at least as severe as the process-wide scope_config::min_log_level.
 **/

#pragma once

#include <cstdint>

namespace xo::pp {
    /** @brief log message severity, ordered least-to-most severe.
     *
     *  Ordering (increasing): never < verbose < chatty < info < warning
     *  < error < severe < always < silent.
     *
     *  @c never   a message with this level is never printed (disabled scope).
     *  @c always  printed for any min_log_level except @c silent.
     *  @c silent  use as min_log_level to suppress all messages.
     **/
    enum class log_level : std::uint8_t {
        never,
        verbose,
        chatty,
        info,
        warning,
        error,
        severe,
        always,
        silent,

        default_level = error
    };

    inline bool operator> (log_level x, log_level y) {
        return static_cast<std::uint8_t>(x) >  static_cast<std::uint8_t>(y);
    }
    inline bool operator>=(log_level x, log_level y) {
        return static_cast<std::uint8_t>(x) >= static_cast<std::uint8_t>(y);
    }
    inline bool operator< (log_level x, log_level y) {
        return static_cast<std::uint8_t>(x) <  static_cast<std::uint8_t>(y);
    }
    inline bool operator<=(log_level x, log_level y) {
        return static_cast<std::uint8_t>(x) <= static_cast<std::uint8_t>(y);
    }
} /*namespace xo::pp*/

/* end log_level.hpp */
