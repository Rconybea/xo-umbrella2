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
            /** fixed-size string representation for exponent of a basis-power-unit **/
            using power_abbrev_type = flatstring<16>;

            /** @defgroup bpu-abbrev-helpers bpu abbrev helpers **/
            ///@{
            /** @brief construct prefix string for unit exponent
             *
             *  Auxiliary function for @ref bpu_abbrev
             **/
            constexpr power_abbrev_type
            flatstring_from_exponent(const power_ratio_type & power)
            {
                if (power.den() == 1) {
                    if (power.num() == 1) {
                        /* for no exponent annotation for power ^1 */
                        return power_abbrev_type::from_chars("");
                    } else {
                        /* e.g. "^-1", "^2" */
                        return (power_abbrev_type::from_flatstring
                                (flatstring_concat(flatstring("^"),
                                                   power_abbrev_type::from_int(power.num()))));
                    }
                } else {
                    /* e.g. "^1/2", "^-1/2" */
                    return (power_abbrev_type::from_flatstring
                            (flatstring_concat(flatstring("^"),
                                               power.to_str<power_abbrev_type::fixed_capacity>())));
                }
            }

            /** @brief construct suffix abbreviation for a basis-power-unit **/
            static constexpr bpu_abbrev_type
            bpu_abbrev(dim native_dim,
                       const scalefactor_ratio_type & scalefactor,
                       const power_ratio_type & power)
            {
                return (bpu_abbrev_type::from_flatstring
                        (flatstring_concat
                         (bu_abbrev(basis_unit(native_dim, scalefactor)),
                          flatstring_from_exponent(power))));
            }
            ///@}
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
            /** @defgroup bpu-ctors bpu constructors **/
            ///@{
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
            ///@}

            static constexpr bpu<Int> unit_power(const basis_unit & bu) {
                return bpu<Int>(bu, power_ratio_type(1,1));
            }

            /** @defgroup bpu-access-methods bpu access methods **/
            ///@{
            /** @brief report this bpu's @ref basis_unit, e.g. @c detail::bu::minute **/
            constexpr const basis_unit & bu() const { return bu_; }
            /** @brief report this bpu's @ref dimension, e.g. @c dimension::time **/
            constexpr dimension native_dim() const { return bu_.native_dim(); }
            /** @brief report this bpu's scale factor, e.g. @c 60/1 for @c detail::bu::minute **/
            constexpr const scalefactor_ratio_type & scalefactor() const { return bu_.scalefactor(); }
            /** @brief report this bpu's exponent,  e.g. @c 3/1 for bpu representing cubic meters **/
            constexpr const power_ratio_type & power() const { return power_; }
            ///@}

            /** @defgroup bpu-methods **/
            ///@{
            /** @brief abbreviation for this dimension
             *
             *  @code
             *  bpu<int64_t>(dim::time,
             *               scalefactor_ratio_type(60,1),
             *               power_ratio_type(-2,1)).abbrev() => "min^-2"
             *  @endcode
             **/
            constexpr bpu_abbrev_type abbrev() const
                {
                    return abbrev::bpu_abbrev(bu_.native_dim_,
                                              bu_.scalefactor_,
                                              power_);
                }

            /** @brief for bpu @c x, @c x.reciprocal() represents dimension of @c 1/x
             *
             *  Example:
             *  @code
             *  constexpr auto x = bpu<int64_t>(dim::time,
             *                                  scalefactor_ratio_type(60,1),
             *                                  power_ratio_type(1));
             *  x.abbrev() => "min"
             *  x.reciprocal().abbrev() => "min^-1"
             *  @endcode
             **/
            constexpr bpu<Int> reciprocal() const {
                return bpu<Int>(bu_.native_dim(), bu_.scalefactor(), power_.negate());
            }

            /** @brief construct bpu representing the same unit, but using @c Int2 to represent exponenct **/
            template <typename Int2>
            constexpr bpu<Int2> to_repr() const {
                return bpu<Int2>(this->native_dim(),
                                 this->scalefactor(),
                                 ratio::ratio<Int2>(power_.num(), power_.den()));
            }
            ///@}

        public: /* need public members so that a basis_unit instance can be a non-type template parameter (a structural type) */
            /** @defgroup bpu-instance-vars **/
            ///@{
            /** @brief this bpu represent a power of this basis unit **/
            struct basis_unit bu_;
            /** @brief this unit represents basis dimension (bu) taken to this power **/
            power_ratio_type power_;
            ///@}
        };

        /** @defgroup bpu-comparison **/
        ///@{
        /** @brief compare bpus @p x and @p y for equality **/
        template <typename Int>
        inline constexpr bool
        operator==(const bpu<Int> & x, const bpu<Int> & y) {
            return ((x.native_dim() == y.native_dim())
                    && (x.scalefactor() == y.scalefactor())
                    && (x.power_ == y.power_));
        }

        /** @brief compare bpus @p x and @p y for inequality **/
        template <typename Int>
        inline constexpr bool
        operator!=(const bpu<Int> & x, const bpu<Int> & y) {
            return ((x.native_dim() != y.native_dim())
                    || (x.scalefactor() != y.scalefactor())
                    || (x.power_ != y.power_));
        }
        ///@}

    } /*namespace qty*/
} /*namespace xo*/

/** end bpu.hpp **/
