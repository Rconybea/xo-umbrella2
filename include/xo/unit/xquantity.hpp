/** @file xquantity.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "quantity_ops.hpp"
#include "scaled_unit.hpp"
#include "natural_unit.hpp"

namespace xo {
    namespace qty {
        /** @class xquantity
         *  @brief represent a scalar quantity with polymorphic units.
         *
         *  - @p Repr type used represent a dimensionless multiple of a natural unit.
         *
         *  Constexpr implementation,  but units are explicitly represented:
         *  @code
         *  sizeof(xquantity) > sizeof(xquantity::repr_type)
         *  @endcode
         *
         *  Explicit unit representation allows introducing units at runtime,
         *  for example in python bindings.
         *  See for example <a href="https://github.com/rconybea/xo-pyutil">xo-pyutil</a>
         *
         *  See @ref quantity for implementation with units established at compile time
         *
         *  Require:
         *  - Repr supports numeric operations (+, -, *, /)
         *  - Repr supports conversion from double.
         **/
        template <typename Repr = double,
                  typename Int = std::int64_t>
        class xquantity {
        public:
            /** @defgroup xquantity-type-traits xquantity type traits **/
            ///@{
            /** @brief runtime representation for this value's scale **/
            using repr_type = Repr;
            /** @brief runtime representation for this value's unit **/
            using unit_type = natural_unit<Int>;
            /** @brief type used for numerator and denominator in basis-unit scalefactor ratios **/
            using ratio_int_type = Int;
            /** @brief double-width type for numerator and denominator of intermediate
             *         scalefactor ratios.  Use to mitigate loss of precision during computation
             *         of conversion factors between units with widely-differing magnitude
             **/
            using ratio_int2x_type = detail::width2x_t<typename unit_type::ratio_int_type>;
            ///@}

        public:
            /** @defgroup xquantity-ctors xquantity constructors **/
            ///@{

            /** create dimensionless, zero quantity **/
            constexpr xquantity()
                : scale_{0}, unit_{natural_unit<Int>()} {}
            /** create quantity representing multiple of @p scale times @p unit **/
            constexpr xquantity(Repr scale,
                                const natural_unit<Int> & unit)
                : scale_{scale}, unit_{unit} {}
            /** create quantity representing multiple of @p scale times @p unit.
             *
             *  Collects outer scalefactors (if any) from @p unit,
             *  so for example:
             *
             *  @code
             *  using namespace xo::qty;
             *  xquantity q(123, u::meter * u::millimeter);
             *
             *  q.scale() --> 0.123
             *  @endcode
             **/
            constexpr xquantity(Repr scale,
                                const scaled_unit<Int> & unit)
                :
                scale_(scale
                       * unit.outer_scale_factor_.template convert_to<double>()
                       * ((unit.outer_scale_sq_ == 1.0)
                          ? 1.0
                          : ::sqrt(unit.outer_scale_sq_))
                    ),
                  unit_{unit.natural_unit_} {}

            ///@}

            /** @defgroup xquantity-constants static xquantity constants **/
            ///@{

            /** false since unit information may be unknown at compile time.
             *  Coordinates with @c quantity::always_constexpr_unit
             **/
            static constexpr bool always_constexpr_unit = false;

            ///@}

            /** @defgroup xquantity-access-methods xquantity access methods **/
            ///@{

            /** get member @ref scale_ **/
            constexpr const repr_type & scale() const { return scale_; }
            /** get member @ref unit_ **/
            constexpr const unit_type & unit() const { return unit_; }

            /** true iff this quantity has no dimension **/
            constexpr bool is_dimensionless() const { return unit_.is_dimensionless(); }

            /** return abbreviation for quantities with this unit **/
            constexpr nu_abbrev_type abbrev() const { return unit_.abbrev(); }

            ///@}

            /** @defgroup xquantity-arithmetic-support**/
            ///@{

            /** create unit quantity with same unit as @c this **/
            constexpr xquantity unit_qty() const { return xquantity(1, unit_); }

            /** create zero quantity with same unit as @c this **/
            constexpr xquantity zero_qty() const { return xquantity(0, unit_); }

            /** create quantity representing reciprocal of @c this **/
            constexpr xquantity reciprocal() const { return xquantity(1.0 / scale_, unit_.reciprocal()); }

            ///@}

            /** @defgroup xquantity-methods xquantity methods **/
            ///@{

            /** create quantity representing the same value,  but in units of @p unit2 **/
            constexpr
            auto rescale(const natural_unit<Int> & unit2) const {
                /* conversion factor from .unit -> unit2*/
                auto rr = detail::su_ratio<ratio_int_type,
                                           ratio_int2x_type>(this->unit_, unit2);

                if (rr.natural_unit_.is_dimensionless()) {
                    repr_type r_scale = (::sqrt(rr.outer_scale_sq_)
                                         * rr.outer_scale_factor_.template convert_to<repr_type>()
                                         * this->scale_);
                    return xquantity(r_scale, unit2);
                } else {
                    return xquantity(std::numeric_limits<repr_type>::quiet_NaN(), unit2);
                }
            }

            /** create quantity representing this value scaled by dimensionless mutliplier @p x **/
            template <typename Dimensionless>
            requires std::is_arithmetic_v<Dimensionless>
            constexpr auto scale_by(Dimensionless x) const {
                return xquantity(x * this->scale_, this->unit_);
            }

            /** create quantity representing this value scaled by dimensionless multiplier @p 1/x **/
            template <typename Dimensionless>
            requires std::is_arithmetic_v<Dimensionless>
            constexpr auto divide_by(Dimensionless x) const {
                return xquantity(this->scale_ / x, this->unit_);
            }

            /** create quantity representing dimensionless numerator @p x divided by this value **/
            template <typename Dimensionless>
            requires std::is_arithmetic_v<Dimensionless>
            constexpr auto divide_into(Dimensionless x) const {
                return xquantity(x / this->scale_, this->unit_.reciprocal());
            }

            /** multiply two @c xquantity values, or a mixed (@c xquantity, @c quantity) pair **/
            template <typename Quantity2>
            static constexpr
            auto multiply(const xquantity & x, const Quantity2 & y) {
                using r_repr_type = std::common_type_t<typename xquantity::repr_type,
                                                       typename Quantity2::repr_type>;
                using r_int_type = std::common_type_t<typename xquantity::ratio_int_type,
                                                      typename Quantity2::ratio_int_type>;
                using r_int2x_type = std::common_type_t<typename xquantity::ratio_int2x_type,
                                                        typename Quantity2::ratio_int2x_type>;

                auto rr = detail::su_product<r_int_type, r_int2x_type>(x.unit(), y.unit());

                r_repr_type r_scale = (::sqrt(rr.outer_scale_sq_)
                                       * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                       * static_cast<r_repr_type>(x.scale())
                                       * static_cast<r_repr_type>(y.scale()));

                return xquantity<r_repr_type, r_int_type>(r_scale,
                                                          rr.natural_unit_);
            }

            /** compute quotient @p x / @p y,  where @p x and @p y are xquantities **/
            template <typename Quantity2>
            static constexpr
            auto divide(const xquantity & x, const Quantity2 & y) {
                using r_repr_type = std::common_type_t<typename xquantity::repr_type,
                                                       typename Quantity2::repr_type>;
                using r_int_type = std::common_type_t<typename xquantity::ratio_int_type,
                                                      typename Quantity2::ratio_int_type>;
                using r_int2x_type = std::common_type_t<typename xquantity::ratio_int2x_type,
                                                        typename Quantity2::ratio_int2x_type>;

                auto rr = detail::su_ratio<r_int_type, r_int2x_type>(x.unit(), y.unit());

                /* note: su_ratio() reports multiplicative outer scaling factors,
                 *       so multiply is correct here
                 */
                r_repr_type r_scale = (::sqrt(rr.outer_scale_sq_)
                                       * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                       * static_cast<r_repr_type>(x.scale())
                                       / static_cast<r_repr_type>(y.scale()));

                return xquantity<r_repr_type, r_int_type>(r_scale,
                                                         rr.natural_unit_);
            }

            /** compute sum @p x + @p y,  where @p x and @p y are xquantities **/
            template <typename Quantity2>
            static constexpr
            auto add(const xquantity & x, const Quantity2 & y) {
                using r_repr_type = std::common_type_t<typename xquantity::repr_type,
                                                       typename Quantity2::repr_type>;
                using r_int_type = std::common_type_t<typename xquantity::ratio_int_type,
                                                      typename Quantity2::ratio_int_type>;
                using r_int2x_type = std::common_type_t<typename xquantity::ratio_int2x_type,
                                                        typename Quantity2::ratio_int2x_type>;

                /* conversion to get y in same units as x:  multiply by y/x */
                auto rr = detail::su_ratio<r_int_type, r_int2x_type>(y.unit(), x.unit());

                if (rr.natural_unit_.is_dimensionless()) {
                    r_repr_type r_scale = (static_cast<r_repr_type>(x.scale())
                                           + (::sqrt(rr.outer_scale_sq_)
                                              * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                              * static_cast<r_repr_type>(y.scale())));

                    return xquantity<r_repr_type, r_int_type>(r_scale, x.unit_.template to_repr<r_int_type>());
                } else {
                    /* units don't match! */
                    return xquantity<r_repr_type, r_int_type>(std::numeric_limits<r_repr_type>::quiet_NaN(),
                                                             x.unit_.template to_repr<r_int_type>());
                }
            }

            /** compute difference @p x - @p y,  where @p x and @p y are xquantities **/
            template <typename Quantity2>
            static constexpr
            auto subtract(const xquantity & x, const Quantity2 & y) {
                using r_repr_type = std::common_type_t<typename xquantity::repr_type,
                                                       typename Quantity2::repr_type>;
                using r_int_type = std::common_type_t<typename xquantity::ratio_int_type,
                                                      typename Quantity2::ratio_int_type>;
                using r_int2x_type = std::common_type_t<typename xquantity::ratio_int2x_type,
                                                        typename Quantity2::ratio_int2x_type>;

                /* conversion to get y in same units as x:  multiply by y/x */
                auto rr = detail::su_ratio<r_int_type, r_int2x_type>(y.unit(), x.unit());

                if (rr.natural_unit_.is_dimensionless()) {
                    r_repr_type r_scale = (static_cast<r_repr_type>(x.scale())
                                           - (::sqrt(rr.outer_scale_sq_)
                                              * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                              * static_cast<r_repr_type>(y.scale())));

                    return xquantity<r_repr_type, r_int_type>(r_scale, x.unit_.template to_repr<r_int_type>());
                } else {
                    /* units don't match! */
                    return xquantity<r_repr_type, r_int_type>(std::numeric_limits<r_repr_type>::quiet_NaN(),
                                                             x.unit_.template to_repr<r_int_type>());
                }
            }

            /** @defgroup xquantity-comparison-support xquantity comparison support methods **/
            ///@{

            /** perform 3-way comparison between @c xquantity values @p x and @p y **/
            template <typename Quantity2>
            static constexpr
            auto compare(const xquantity & x, const Quantity2 & y) {
                xquantity y2 = y.rescale(x.unit_);

                return x.scale() <=> y2.scale();
            }

            ///@}

            /** @defgroup xquantity-operators xquantity operators **/
            ///@{
            xquantity operator-() const {
                return xquantity(-scale_, unit_);
            }

            /* also works with Quantity2 = double, int, .. */
            template <typename Quantity2>
            xquantity & operator*= (const Quantity2 & x) {
                *this = *this * x;
                return *this;
            }

            /* also works with Quantity2 = double, int, .. */
            template <typename Quantity2>
            xquantity & operator/= (const Quantity2 & x) {
                *this = *this / x;
                return *this;
            }

            ///@}

        private:
            /** @defgroup xquantity-instance-vars **/
            ///@{

            /** quantity represents this multiple of a unit amount **/
            Repr scale_ = Repr{};
            /** unit for this quantity **/
            natural_unit<Int> unit_;

            ///@}
        }; /*xquantity*/

        /** note: won't have constexpr result until c++26 (when @c sqrt(), @c pow() are constexpr)
         **/
        template <typename Repr = double,
                  typename Int = std::int64_t>
        inline constexpr xquantity<Repr, Int>
        unit_qty(const scaled_unit<Int> & u)
        {
            return xquantity<Repr, Int>
                (u.outer_scale_factor_.template convert_to<double>() * ::sqrt(u.outer_scale_sq_),
                 u.natural_unit_);
        }

        /** note: won't have constexpr result until c++26 (when @c sqrt(), @c pow() are constexpr)
         **/
        template <typename Repr = double,
                  typename Int = std::int64_t>
        inline constexpr xquantity<Repr, Int>
        natural_unit_qty(const natural_unit<Int> & nu) {
            return xquantity<Repr, Int>(1.0, nu);
        }

        /** note: won't have constexpr result until c++26 (when @c sqrt(), @c pow() are constexpr)
         **/
        template <typename Q1, typename Q2>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && (!Q1::always_constexpr_unit || !Q2::always_constexpr_unit))
        constexpr auto
        operator* (const Q1 & x, const Q2 & y)
        {
            return Q1::multiply(x, y);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator* (double x, const Quantity & y)
        {
            return y.scale_by(x);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator* (const Quantity & x, double y)
        {
            return x.scale_by(y);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename Quantity2>
        requires quantity_concept<Quantity> && quantity_concept<Quantity2>
        constexpr auto
        operator/ (const Quantity & x, const Quantity2 & y)
        {
            return Quantity::divide(x, y);
        }

        /** note: doesn not require unit scaling, so constexpr with c++23 **/
        template <typename Quantity, typename Dimensionless>
        requires quantity_concept<Quantity> && std::is_arithmetic_v<Dimensionless>
        constexpr auto
        operator/ (const Quantity & x, Dimensionless y)
        {
            return x.divide_by(y);
        }

        /** note: doesn not require unit scaling, so constexpr with c++23 **/
        template <typename Dimensionless, typename Quantity>
        requires std::is_arithmetic_v<Dimensionless> && quantity_concept<Quantity>
        constexpr auto
        operator/ (Dimensionless x, const Quantity & y)
        {
            return y.divide_into(x);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename Quantity2>
        requires quantity_concept<Quantity> && quantity_concept<Quantity2>
        constexpr auto
        operator+ (const Quantity & x, const Quantity2 & y)
        {
            return Quantity::add(x, y);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator+ (const Quantity & x, double y)
        {
            return x + Quantity(y, nu::dimensionless);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator+ (double x, const Quantity & y)
        {
            return Quantity(x, nu::dimensionless) + y;
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename Quantity2>
        requires (quantity_concept<Quantity>
                  && quantity_concept<Quantity2>)
        constexpr auto
        operator- (const Quantity & x, const Quantity2 & y)
        {
            return Quantity::subtract(x, y);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator- (const Quantity & x, double y)
        {
            return x - Quantity(y, nu::dimensionless);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator- (double x, const Quantity & y)
        {
            return Quantity(x, nu::dimensionless) - y;
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator== (const Quantity & x, double y)
        {
            return (x == Quantity(y, nu::dimensionless));
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator== (double x, const Quantity & y)
        {
            return (Quantity(x, nu::dimensionless) == y);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, double>
        requires quantity_concept<Quantity>
        constexpr auto
        operator<=> (const Quantity & x, double y)
        {
            return Quantity::compare(x, Quantity(y, nu::dimensionless));
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, double>
        requires quantity_concept<Quantity>
        constexpr auto
        operator<=> (double x, const Quantity & y)
        {
            return Quantity::compare(Quantity(x, nu::dimensionless), y);
        }

        namespace unit {
            constexpr auto nanogram = natural_unit_qty(nu::nanogram);
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end xquantity.hpp **/
