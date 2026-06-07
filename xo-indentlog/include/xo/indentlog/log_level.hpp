/* @file log_level.hpp */

#pragma once

#include <iostream>
#include <cstdint>

namespace xo {
    enum class log_level : std::uint8_t {
        /* control log message severity
         *   silent > always > severe > error > warning > info > chatty > never
         *
         * never:
         *   used internally e.g. by XO_ENTER1()
         *   a log message with this severity will never be printed
         *
         * always:
         *   use with XO_ENTER1():
         *     scope log(XO_ENTER1(always, mydebug_flag));
         *   to log message whenever mydebug_flag is true (for any .min_log_level except silent)
         *
         * silent:
         *   use in log_config to suppress all log messages
         */
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
    }; /*log_level*/

    inline bool
    operator>(log_level x, log_level y) {
        return (static_cast<std::uint32_t>(x) > static_cast<std::uint32_t>(y));
    }

    inline bool
    operator>=(log_level x, log_level y) {
        return (static_cast<std::uint32_t>(x) >= static_cast<std::uint32_t>(y));
    }

    inline bool
    operator<(log_level x, log_level y) {
        return (static_cast<std::uint32_t>(x) < static_cast<std::uint32_t>(y));
    }

    inline bool
    operator<=(log_level x, log_level y) {
        return (static_cast<std::uint32_t>(x) <= static_cast<std::uint32_t>(y));
    }

    inline std::ostream &
    operator<<(std::ostream & os,
               log_level x) {
        switch(x) {
        case log_level::never: os << "never"; break;
        case log_level::verbose: os << "verbose"; break;
        case log_level::chatty: os << "chatty"; break;
        case log_level::info: os << "info"; break;
        case log_level::warning: os << "warning"; break;
        case log_level::error: os << "error"; break;
        case log_level::severe: os << "severe"; break;
        case log_level::always: os << "always"; break;
        case log_level::silent: os << "silent"; break;
            //default: os << "???"; break;
        }
        return os;
    } /* operator<<*/
} /*namespace xo*/

/* end log_level.hpp */
