/* @file log_level.hpp */

#include <cstdint>

namespace xo {
    enum class log_level : std::uint32_t {
        /* control log message severity
         *   silent > severe > error > warning > info > chatty
         */
        chatty,
        info,
        warning,
        error,
        severe,
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
