/** @file bpu.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "basis_unit.hpp"
#include "bu_store.hpp"

namespace xo {
    namespace qty {
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
                         (bu_abbrev(basis_unit(native_dim, scalefactor)),
                          flatstring_from_exponent(power.num(), power.den()))));
            }
        }

        /** @class bpu
         *
         *  @brief represent product of a compile-time scale-factor with a rational power of a native unit
         **/
        template<typename Int>
        struct bpu {
        public:
            using ratio_int_type = Int;

        public:
            constexpr bpu() = default;
            constexpr bpu(const basis_unit & bu,
                          const power_ratio_type & power)
                : bu_{bu},
                  power_{power}
                {}
            constexpr bpu(dim native_dim,
                          const scalefactor_ratio_type & scalefactor,
                          const power_ratio_type & power)
                : bu_(native_dim, scalefactor),
                  power_{power}
                {}

            static constexpr bpu<Int> unit_power(const basis_unit & bu) {
                return bpu<Int>(bu, power_ratio_type(1,1));
            }

            constexpr dimension native_dim() const { return bu_.native_dim(); }
            constexpr const scalefactor_ratio_type & scalefactor() const { return bu_.scalefactor(); }
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
                    return abbrev::bpu_abbrev(bu_.native_dim_,
                                              bu_.scalefactor_,
                                              power_);
                }

            /* for bpu x, x.reciprocal() represents dimension of 1/x */
            constexpr bpu<Int> reciprocal() const {
                return bpu<Int>(bu_.native_dim(), bu_.scalefactor(), power_.negate());
            }

            template <typename Int2>
            constexpr bpu<Int2> to_repr() const {
                return bpu<Int2>(this->native_dim(),
                                 this->scalefactor(),
                                 ratio::ratio<Int2>(power_.num(), power_.den()));
            }

        public: /* need public members so that a basis_unit instance can be a non-type template parameter (a structural type) */
            /** @brief this bpu represent a power of this basis unit **/
            basis_unit bu_;
            /** @brief this unit represents basis dimension (bu) taken to this power **/
            power_ratio_type power_;
        };

        template <typename Int>
        constexpr auto make_unit_power(const basis_unit & bu) {
            return bpu<Int>::unit_power(bu);
        }

        template <typename Int>
        inline constexpr bool
        operator==(const bpu<Int> & x, const bpu<Int> & y) {
            return ((x.native_dim() == y.native_dim())
                    && (x.scalefactor() == y.scalefactor())
                    && (x.power_ == y.power_));
        }

        template <typename Int>
        inline constexpr bool
        operator!=(const bpu<Int> & x, const bpu<Int> & y) {
            return ((x.native_dim() != y.native_dim())
                    || (x.scalefactor() != y.scalefactor())
                    || (x.power_ != y.power_));
        }

    } /*namespace qty*/
} /*namespace xo*/

/** end bpu.hpp **/
