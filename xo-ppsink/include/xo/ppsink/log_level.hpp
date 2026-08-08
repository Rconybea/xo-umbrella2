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
#include <string_view>

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

    /** @brief name of @p x, e.g. "chatty".
     *
     *  ostream-free by design, like the rest of this header; see
     *  log_level_ostream.hpp for @c os << level .  The legacy xo-indentlog
     *  log_level had an operator<< and no name accessor; splitting them lets a
     *  caller name a level without pulling in <ostream>.
     **/
    inline constexpr std::string_view
    log_level_name(log_level x) noexcept {
        switch (x) {
        case log_level::never:   return "never";
        case log_level::verbose: return "verbose";
        case log_level::chatty:  return "chatty";
        case log_level::info:    return "info";
        case log_level::warning: return "warning";
        case log_level::error:   return "error";
        case log_level::severe:  return "severe";
        case log_level::always:  return "always";
        case log_level::silent:  return "silent";
        }

        return "?log_level";
    }

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
