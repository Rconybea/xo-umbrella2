/** @file native_unit.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "dimension.hpp"
#include "xo/flatstring/flatstring.hpp"

namespace xo {
    namespace qty {
        using native_unit2_abbrev_type = flatstring<8>;

        /** @class native_unit
         *
         *  @brief Represent a native (built-in) unit.
         *
         *  A basis_unit is expressed as a multiple of a native_unit
         *
         **/
        struct native_unit {
        public:
            constexpr native_unit(dimension native_dim,
                                  const native_unit2_abbrev_type & abbrev_str)
                : native_dim_{native_dim},
                  abbrev_str_{abbrev_str}
                {}

            constexpr dimension native_dim() const { return native_dim_; }
            constexpr const native_unit2_abbrev_type & abbrev_str() const { return abbrev_str_; }

        private:
            dimension native_dim_;
            native_unit2_abbrev_type abbrev_str_;
        };

        static constexpr native_unit native_unit2_v[n_dim] = {
            native_unit(dimension::mass,     native_unit2_abbrev_type::from_chars("g")),
            native_unit(dimension::distance, native_unit2_abbrev_type::from_chars("m")),
            native_unit(dimension::time,     native_unit2_abbrev_type::from_chars("s")),
            native_unit(dimension::currency, native_unit2_abbrev_type::from_chars("ccy")),
            native_unit(dimension::price,    native_unit2_abbrev_type::from_chars("px")),
        };

    } /*namespace qty*/
} /*namespace xo*/

/** end native_unit.hpp **/
