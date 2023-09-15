/* @file log_config.hpp */

#pragma once

#include "function.hpp"
#include "nestlog/color.hpp"
#include <cstdint>

namespace xo {
    /* Tag here b/c we want header-only library */
    template <typename Tag>
    struct log_config_impl {
        /* spaces per nesting level */
        static std::uint32_t indent_width;
        /* max #of spaces to introduce when indenting */
        static std::uint32_t max_indent_width;
        /* if true enable explicit nesting level display [nnn] */
        static bool nesting_level_enabled;
        /* color to use for explicit nesting level */
        static std::uint32_t nesting_level_color;
        /* display style for function names.  FS_Simple|FS_Pretty|FS_Streamlined */
        static function_style style;
        /* color encoding */
        static color_encoding encoding;
        /* color to use for function name,  on entry/exit (xo::scope creation/destruction)
         * (ansi color codes,  see Select Graphics Rendition subset)
         */
        static std::uint32_t function_entry_color;
        static std::uint32_t function_exit_color;
        /* if true,  append [file:line] to output */
        static bool location_enabled;
        /* when .location_enabled,  write [file:line] starting this many chars from left margin */
        static std::uint32_t location_tab;
        /* color to use for code location */
        static std::uint32_t code_location_color;
    }; /*log_config_impl*/

    template <typename Tag>
    std::uint32_t
    log_config_impl<Tag>::indent_width = 1;

    template <typename Tag>
    std::uint32_t
    log_config_impl<Tag>::max_indent_width = 32;

    template <typename Tag>
    bool
    log_config_impl<Tag>::nesting_level_enabled = true;

    template <typename Tag>
    std::uint32_t
    log_config_impl<Tag>::nesting_level_color = 195;

    template <typename Tag>
    function_style
    log_config_impl<Tag>::style = FS_Streamlined;

    template <typename Tag>
    color_encoding
    log_config_impl<Tag>::encoding = CE_Ansi;

    template <typename Tag>
    std::uint32_t
    log_config_impl<Tag>::function_entry_color = 34;

    template <typename Tag>
    std::uint32_t
    log_config_impl<Tag>::function_exit_color = 32;

    template <typename Tag>
    bool
    log_config_impl<Tag>::location_enabled = true;

    template <typename Tag>
    std::uint32_t
    log_config_impl<Tag>::location_tab = 80;

    template <typename Tag>
    std::uint32_t
    log_config_impl<Tag>::code_location_color = 31;

    using log_config = log_config_impl<class log_config_tag>;
} /*namespace xo*/

/* end log_config.hpp */
