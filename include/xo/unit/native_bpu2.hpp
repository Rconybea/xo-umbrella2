/** @file native_bpu2.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "basis_unit2.hpp"

namespace xo {
    namespace unit {
        using bpu2_abbrev_type = flatstring<24>;

        using power_ratio_type = xo::ratio::ratio<std::int64_t>;

        /** @class native_bpu2
         *
         *  @brief represent product of a compile-time scale-factor with a rational power of a native unit
         *
         *  Example:
         *  native_bpu<universal::time, ratio<1>, ratio<-1,2>> represents unit of 1/sqrt(t)
         **/
        template<typename Int>
        struct bpu2 : basis_unit2 {
        public:
            constexpr bpu2(power_ratio_type power,
                           dim native_dim,
                           scalefactor_ratio_type scalefactor)
                : basis_unit2(native_dim, scalefactor),
                  power_{power}
                {}

            constexpr const power_ratio_type & power() const { return power_; }

            /** @brief this unit represents native dimension taken to this power **/
            power_ratio_type power_;
        };

        template <
            dim BasisDim,
            std::int64_t InnerScaleNum, std::int64_t InnerScaleDen,
            std::int64_t PowerNum, std::int64_t PowerDen
            >
        constexpr bpu2_abbrev_type
        bpu2_assemble_abbrev_helper()
        {
            return flatstring_concat
                (units::scaled_native_unit2_abbrev_v<BasisDim, InnerScaleNum, InnerScaleDen>,
                 flatstring_from_exponent<PowerNum, PowerDen>());
        };

        template < typename BPU >
        constexpr auto bpu2_assemble_abbrev(const BPU & bpu) {
            // bpu.power(), bpu.native_dim(), bpu.scalefactor()

            return bpu2_assemble_abbrev_helper<
                bpu.native_dim(),
                bpu.scalefactor().num(), bpu.scalefactor().den(),
                bpu.power().num(), bpu.power().den()>;
        };
    } /*namespace unit*/
} /*namespace xo*/


/** end native_bpu2.hpp **/
