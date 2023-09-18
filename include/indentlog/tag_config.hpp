/* @file tag_config.hpp */

#pragma once

#include "color.hpp"
#include <cstdint>

namespace xo {
    /* Tag here b/c we want header-only library */
    template <typename Tag>
    struct tag_config_impl {
        /* color encoding */
        static color_encoding encoding;
        /* color to use for tags
         *    os << tag("foo", foovalue)
         * to produces output like
         *    :foo foovalue
         * with :foo using .tag_color
         */
        static std::uint32_t tag_color;
    }; /*tag_config_impl*/

    template <typename Tag>
    color_encoding
    tag_config_impl<Tag>::encoding = color_encoding::xterm;

    template <typename Tag>
    std::uint32_t
    tag_config_impl<Tag>::tag_color = 245;

    using tag_config = tag_config_impl<class tag_config_tag>;
} /*namespace xo*/

/* end tag_config.hpp */
