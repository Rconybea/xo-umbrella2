/* @file log_config.hpp */

#pragma once

#include "function.hpp"
#include <cstdint>

namespace xo {
    /* Tag here b/c we want header-only library */
    template <typename Tag>
    struct log_config_impl {
        /* spaces per indent level */
        static std::uint32_t indent_width;
        /* display style for function names.  FS_Simple|FS_Pretty|FS_Streamlined */
        static function_style style;
        /* if true,  append [file:line] to output */
        static bool location_enabled;
        /* when .location_enabled,  write [file:line] starting this many chars from left margin */
        static std::uint32_t location_tab;
    }; /*log_config_impl*/

    template <typename Tag>
    std::uint32_t
    log_config_impl<Tag>::indent_width = 1;

    template <typename Tag>
    function_style
    log_config_impl<Tag>::style = FS_Streamlined;

    template <typename Tag>
    bool
    log_config_impl<Tag>::location_enabled = true;

    template <typename Tag>
    std::uint32_t
    log_config_impl<Tag>::location_tab = 80;

    using log_config = log_config_impl<class log_config_tag>;
} /*namespace xo*/

/* end log_config.hpp */
