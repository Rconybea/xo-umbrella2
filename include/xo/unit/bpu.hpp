/** @file bpu.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/indentlog/print/tag.hpp"
#include "basis_unit.hpp"
#include "dim_iostream.hpp"

namespace xo {
    namespace qty {
        using bpu_abbrev_type = flatstring<24>;

        using power_ratio_type = xo::ratio::ratio<std::int64_t>;

        namespace abbrev {
            using power_abbrev_type = flatstring<16>;

            constexpr power_abbrev_type
            flatstring_from_exponent(std::int64_t num,
                                     std::int64_t den)
            {
                if (den == 1) {
                    if (num == 1) {
                        return power_abbrev_type::from_chars("");
                    } else {
                        return (power_abbrev_type::from_flatstring
                                (flatstring_concat(flatstring("^"),
                                                   power_abbrev_type::from_int(num))));
                    }
                } else {
                    return (power_abbrev_type::from_flatstring
                            (flatstring_concat(flatstring("^"),
                                               xo::ratio::make_ratio(num, den)
                                               .to_str<power_abbrev_type::fixed_capacity>())));
                }
            }

            static constexpr bpu_abbrev_type
            bpu_abbrev(dim native_dim,
                        const scalefactor_ratio_type & scalefactor,
                        const power_ratio_type & power)
            {
                return (bpu_abbrev_type::from_flatstring
                        (flatstring_concat
                         (basis_unit2_abbrev(native_dim, scalefactor),
                          flatstring_from_exponent(power.num(), power.den()))));
            }
        }

        /** @class native_bpu2
         *
         *  @brief represent product of a compile-time scale-factor with a rational power of a native unit
         *
         *  Example:
         *  native_bpu<universal::time, ratio<1>, ratio<-1,2>> represents unit of 1/sqrt(t)
         **/
        template<typename Int>
        struct bpu : basis_unit {
        public:
            using ratio_int_type = Int;

        public:
            constexpr bpu() = default;
            constexpr bpu(const basis_unit & bu,
                          const power_ratio_type & power)
                : basis_unit{bu},
                  power_{power}
                {}
            constexpr bpu(dim native_dim,
                           const scalefactor_ratio_type & scalefactor,
                           const power_ratio_type & power)
                : basis_unit(native_dim, scalefactor),
                  power_{power}
                {}

            static constexpr bpu<Int> unit_power(const basis_unit & bu) {
                return bpu<Int>(bu, power_ratio_type(1,1));
            }

            constexpr const power_ratio_type & power() const { return power_; }

            /** @brief abbreviation for this dimension
             *
             *  @code
             *  bpu2<int64_t>(dim::time,
             *                scalefactor_ratio_type(60,1),
             *                power_ratio_type(-2,1)).abbrev() => "min^-2"
             *  @endcode
             **/
            constexpr bpu_abbrev_type abbrev() const
                {
                    return abbrev::bpu_abbrev(native_dim_,
                                               scalefactor_,
                                               power_);
                }

            /* for bpu x, x.reciprocal() represents dimension of 1/x */
            constexpr bpu<Int> reciprocal() const {
                return bpu<Int>(native_dim(), scalefactor(), power_.negate());
            }

            /** @brief this unit represents native dimension taken to this power **/
            power_ratio_type power_;
        };

        template <typename Int>
        constexpr auto make_unit_power(const basis_unit & bu) {
            return bpu<Int>::unit_power(bu);
        }

    } /*namespace qty*/
} /*namespace xo*/

/** end bpu.hpp **/
