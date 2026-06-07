/* @file dimension.hpp */

#pragma once

#ifdef __APPL__
# include <vector>  // for std::size_t when building with clang18
#endif
#include <cstdint>

namespace xo {
    namespace qty {
        /** @enum dimension
         *  @brief represent an abstract dimension.
         *
         *  *xo-unit* units are expressed as a cartesian product
         *  of powers of these dimensions.
         **/
        enum class dimension {
            /** sentinel value.  not a dimension **/
            invalid = -1,

            /** weight.  native unit = 1 gram **/
            mass,
            /** distance.  native unit = 1 meter **/
            distance,
            /** time. native unit = 1 second **/
            time,
            /** a currency amount. native unit depends on actual currency.
             *  For USD: one US dollar.
             *
             *  NOTE: multicurrency work not supported by *xo-unit*.
             *  - (1usd + 1eur) is well-defined.
             *  - (1sec + 1m) is not.
             **/
            currency,
            /** A screen price.
             *  The interpretation of prices is highly context dependent;
             *  expect useful to bucket separately from currenty amounts.
             **/
            price,

            /** not a dimension.  comes last, counts entries **/
            n_dim
        };

        using dim = dimension;

        /** @brief string value for a dimension enum **/
        inline const char *
        dim2str(dimension x)
        {
            switch(x) {
            case dimension::mass:     return "mass";
            case dimension::distance: return "distance";
            case dimension::time:     return "time";
            case dimension::currency: return "currency";
            case dimension::price:    return "price";
            default: break;
            }
            return "?dim";
        }

        /** @brief number of built-in dimensions, convenient for array sizing **/
        static constexpr std::uint64_t n_dim = static_cast<std::uint64_t>(dimension::n_dim);
    } /*namespace qty*/
} /*namespace xo*/

/* end dimension.hpp */
