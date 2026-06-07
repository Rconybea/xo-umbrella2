/** @file width2x.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "natural_unit.hpp"
#include <cstdint>

namespace xo {
    namespace qty {
        namespace detail {
            template <typename Int>
            struct width2x;

            template <>
            struct width2x<std::int16_t> {
                using type = std::int32_t;
            };

            template <>
            struct width2x<std::int32_t> {
                using type = std::int64_t;
            };

            template <>
            struct width2x<std::int64_t> {
                using type = __int128_t;
            };

            template <typename Int>
            using width2x_t = width2x<Int>::type;
        }

    } /*namespace qty*/
} /*namespace xo*/

/** end width2x.hpp **/
