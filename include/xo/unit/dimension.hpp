/* @file dimension.hpp */

#pragma once

#include <cstdint>

namespace xo {
    namespace qty {
        enum class dim {
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
             *  NOTE: unit system isn't suitable for multicurrency work:
             *        (1usd + 1eur) is well-defined,  but (1sec + 1m) is not.
             **/
            currency,
            /** a screen price **/
            price,

            /** comes last, counts entries **/
            n_dim
        };

        inline const char *
        dim2str(dim x)
        {
            switch(x) {
            case dim::mass:     return "mass";
            case dim::distance: return "distance";
            case dim::time:     return "time";
            case dim::currency: return "currency";
            case dim::price:    return "price";
            default: break;
            }
            return "?dim";
        }

        static constexpr std::size_t n_dim = static_cast<std::size_t>(dim::n_dim);
    } /*namespace qty*/
} /*namespace xo*/

/* end dimension.hpp */
