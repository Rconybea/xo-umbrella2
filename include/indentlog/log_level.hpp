/* @file log_level.hpp */

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

} /*namespace xo*/

/* end log_level.hpp */
