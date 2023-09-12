/* @file log_config.hpp */

#pragma once

#include <cstdint>

namespace xo {
    /* Tag here b/c we want header-only library */
    template <typename Tag>
    struct log_config_impl {
        /* spaces per indent level */
        static std::uint32_t indent_width;
    }; /*log_config_impl*/

    template <typename Tag>
    std::uint32_t
    log_config_impl<Tag>::indent_width = 1;

    using log_config = log_config_impl<class log_config_tag>;
} /*namespace xo*/

/* end log_config.hpp */
