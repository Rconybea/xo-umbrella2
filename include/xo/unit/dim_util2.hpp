/** @file dim_util2.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "dim_util.hpp"
#include "xo/flatstring/flatstring.hpp"

namespace xo {
    namespace unit {
        using native_unit2_abbrev_type = flatstring<8>;

        /** @class native_unit2
         *
         *  @brief Represent a native (built-in) unit.
         *
         *  A basis_unit2 is expressed as a multiple of a native_unit2
         *
         **/
        struct native_unit2 {
        public:
            constexpr native_unit2(dim native_dim,
                                   const native_unit2_abbrev_type & abbrev_str)
                : native_dim_{native_dim},
                  abbrev_str_{abbrev_str}
                {}

            constexpr dim native_dim() const { return native_dim_; }
            constexpr const native_unit2_abbrev_type & abbrev_str() const { return abbrev_str_; }

        private:
            dim native_dim_;
            native_unit2_abbrev_type abbrev_str_;
        };

        static constexpr native_unit2 native_unit2_v[n_dim] = {
            native_unit2(dim::mass,     native_unit2_abbrev_type::from_chars("g")),
            native_unit2(dim::distance, native_unit2_abbrev_type::from_chars("m")),
            native_unit2(dim::time,     native_unit2_abbrev_type::from_chars("s")),
            native_unit2(dim::currency, native_unit2_abbrev_type::from_chars("ccy")),
            native_unit2(dim::price,    native_unit2_abbrev_type::from_chars("px")),
        };

    } /*namespace unit*/
} /*namespace xo*/



/** end dim_util2.hpp **/
