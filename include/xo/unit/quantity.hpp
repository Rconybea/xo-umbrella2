/** @file quantity.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "quantity_ops.hpp"
#include "natural_unit.hpp"
#include "scaled_unit.hpp"

namespace xo {
    namespace qty {
        /** @class quantity
         *
         *  @brief represent a scalar quantity with associated units.
         *
         *  - @p NaturalUnit is a non-type template paramoeter
         *    identifying a unit used for this quantity.
         *    In *xo-unit* it will be an instance of @c natural_unit
         *  - @p Repr is a type used to represent a multiple
         *    of @p NaturalUnit.
         *
         *  Enforce dimensional consistency at compile time.
         *  sizeof(quantity) == sizeof(Repr).
         *
         *  A quantity's runtime state consists of exactly one @p Repr instance:
         *  @code
         *  sizeof(quantity<NaturalUnit, Repr>) == sizeof(Repr)
         *  @endcode
         **/
        template <
            auto ScaledUnit,
            typename Repr = double>
        requires (ScaledUnit.is_natural() && ScaledUnit.is_scaled_unit_type())
        class quantity {
        public:
            /** @defgroup quantity-type-traits quantity type traits **/
            ///@{
            /** @brief runtime representation for value of this type **/
            using repr_type = Repr;
            /** @brief type used to represent unit information */
            using unit_type = decltype(ScaledUnit);
            /** @brief type used for numerator and denominator in basis-unit scalefactor ratios */
            using ratio_int_type = unit_type::ratio_int_type;
            /** @brief double-width type used for numerator and denominator of intermediate
             *         scalefactor ratios.  Used to mitigate loss of precision during computation
             *         of conversion factors between units with widely-differing magnitude
             **/
            using ratio_int2x_type = detail::width2x_t<typename unit_type::ratio_int_type>;
            ///@}

        public:
            /** @defgroup quantity-ctors quantity constructors**/
            ///@{
            /** @brief create a zero amount with dimension @c ScaledUnit **/
            constexpr quantity() : scale_{0} {}
            /** @brief create a quantity representing @p scale @c ScaledUnits **/
            explicit constexpr quantity(Repr scale) : scale_{scale} {}
            ///@}

            /** @defgroup quantity-constants static quantity constants **/
            ///@{
            /** @brief Use to distinguish @ref quantity from xquantity instances.
             *
             *  Useful in c++ template resolution.
             **/
            static constexpr bool always_constexpr_unit = true;
            ///@}

            /** @defgroup quantity-access-methods quantity access methods **/
            ///@{
            /** @brief value of @c scale_ in quantity representing amount (@c scale_ * @c s_unit) **/
            constexpr const repr_type & scale() const { return scale_; }
            /** @brief s_unit in quantity representing amount (@c scale_ * @c s_unit) **/
            constexpr const unit_type & unit() const { return s_scaled_unit; }

            /** @brief true iff this quantity represents a dimensionless value **/
            constexpr bool is_dimensionless() const {
                return s_scaled_unit.is_dimensionless();
            }
            ///@}

            // unit_qty
            // zero_qty

            /** @defgroup quantity-arithmetic-support **/
            ///@{
            constexpr
            auto reciprocal() const {
                return quantity<s_scaled_unit.reciprocal(),
                                repr_type>(1.0 / scale_);
            }
            ///@}

            /** @defgroup quantity-unit-conversion **/
            ///@{

            /** create equivalent quantity using scale representation @p Repr2 instead of @c Repr **/
            template <typename Repr2>
            constexpr
            auto with_repr() const {
                return quantity<s_scaled_unit, Repr2>(scale_);
            }

            /** create equivalent quantity expressed as a multiple of @p NaturalUnit2
             *  instead of @ref s_unit
             **/
            template <natural_unit<ratio_int_type> NaturalUnit2>
            constexpr
            auto rescale() const {
                /* conversion factor from .unit -> unit2*/
                auto rr = detail::su_ratio<ratio_int_type,
                                           ratio_int2x_type>(s_scaled_unit.natural_unit_,
                                                             NaturalUnit2);

                if (rr.natural_unit_.is_dimensionless()) {
                    repr_type r_scale = (((rr.outer_scale_sq_ == 1.0)
                                          ? 1.0
                                          : ::sqrt(rr.outer_scale_sq_))
                                         * rr.outer_scale_factor_.template convert_to<repr_type>()
                                         * this->scale_);
                    return quantity<NaturalUnit2, Repr>(r_scale);
                } else {
                        return quantity<NaturalUnit2, Repr>(std::numeric_limits<repr_type>::quiet_NaN());
                }
            }

            /** create equivalent quantity expressed as as multiple of @p ScaledUnit2
             *  instead of @ref s_unit
             **/
            template <scaled_unit<ratio_int_type> ScaledUnit2>
            constexpr
            auto rescale_ext() const {
                /* conversion factor from .unit -> unit2*/
                auto rr = detail::su_ratio<ratio_int_type,
                                           ratio_int2x_type>(s_scaled_unit.natural_unit_,
                                                             ScaledUnit2.natural_unit_);

                if (rr.natural_unit_.is_dimensionless()) {
                    /* NOTE: test for unit .outer_scale_sq values to get constexpr result with c++23
                     *       and integer dimension powers.
                     *
                     * NOTE: we don't intend to support mixed-unit quantities.
                     *       If we change intention, will need to take into account
                     *       (s_scaled_unit.outer_scale_factor_, s_scaled_unit.outer_scale_sq_)
                     */
                    repr_type r_scale = ((((rr.outer_scale_sq_ == 1.0)
                                           && (ScaledUnit2.outer_scale_sq_ == 1.0))
                                          ? 1.0
                                          : ::sqrt(rr.outer_scale_sq_ / ScaledUnit2.outer_scale_sq_))
                                       * rr.outer_scale_factor_.template convert_to<repr_type>()
                                         * this->scale_
                                         / ScaledUnit2.outer_scale_factor_.template convert_to<repr_type>());
                    return quantity<ScaledUnit2, Repr>(r_scale);
                } else {
                    return quantity<ScaledUnit2, Repr>(std::numeric_limits<repr_type>::quiet_NaN());
                }
            }
            ///@}

            /** create quantity representing this amount multiplied by dimensionless value @p x
             *
             *  @pre x must be an arithmetic type such as @c int or @c double
             **/
            template <typename Dimensionless>
            requires std::is_arithmetic_v<Dimensionless>
            constexpr auto scale_by(Dimensionless x) const {
                using r_repr_type = std::common_type_t<repr_type, Dimensionless>;

                return quantity<s_scaled_unit, r_repr_type>(x * this->scale_);
            }

            // divide_by
            // divide_into

            // divide
            // add
            // subtract

            /** @defgroup quantity-comparison-support **/
            ///@{
            /** compare two @c quantity instances, under three-way comparison **/
            template <typename Quantity2>
            static constexpr
            auto compare(const quantity &x, const Quantity2 & y) {
                quantity y2 = y.template rescale_ext<s_scaled_unit>();

                return x.scale() <=> y2.scale();
            }
            ///@}

            // operator-
            // operator+=
            // operator-=
            // operator*=
            // operator/=

            /** **/
            constexpr nu_abbrev_type abbrev() const {
                return s_scaled_unit.natural_unit_.abbrev();
            }

            /** @defgroup quantity-assignment quantity assignment operators **/
            ///@{

            /** assignment from quantity with identical units **/
            quantity & operator=(const quantity & x) {
                this->scale_ = x.scale_;
                return *this;
            }

            /** assignment from quantity with compatible units **/
            template <typename Q2>
            requires(quantity_concept<Q2>
                     && Q2::always_constexpr_unit)
            quantity & operator=(const Q2 & x) {
                auto x2 = x.template rescale_ext<s_scaled_unit>();

                this->scale_ = x2.scale();

                return *this;
            }

            ///@}

            /** @defgroup quantity-unit-conversion **/
            ///@{

            /** */
            template <typename Q2>
            requires(quantity_concept<Q2>
                     && Q2::always_constexpr_unit)
            constexpr operator Q2() const {
                return this->template rescale_ext<Q2::s_scaled_unit>().template with_repr<typename Q2::repr_type>();
            }

            /** For dimensionless quantities: convert to underlying scale value
             *
             *  Not present for dimensioned quantities.
             **/
            constexpr operator Repr() const
                requires (ScaledUnit.is_dimensionless())
                {
                    return scale_;
                }

            ///@}

        public: /* need public members so that instance can be a non-type template parameter (is a structural type) */
            /** @defgroup quantity-static-vars **/
            ///@{

            /** @brief unit for quantity of this type. Determined at compile-time **/
            static constexpr scaled_unit<ratio_int_type> s_scaled_unit = ScaledUnit;

            ///@}

            /** @defgroup quantity-instance-vars **/
            ///@{

            /** quantity represents this multiple of @ref s_scaled_unit
             *
             *  Public to avoid disqualifying @c quantity as a 'structural type';
             *  prerequisite for using a @c quantity instance as a non-type template parameter
             **/
            Repr scale_ = Repr{};

            ///@}
        };

        ///@{

        /**
         *
         **/
        template <typename Quantity, typename Int, typename Int2x>
        constexpr auto
        rescale(const Quantity & x,
                const scaled_unit<Int, Int2x> & su) {
            return x.template rescale<su>();
        }

        ///@}

        namespace detail {
            struct quantity_util {
                /* parallel implementation to xquantity<Repr, Int> multiply,
                 * but return type will have dimension computed at compile-time
                 */
                template <typename Q1, typename Q2>
                requires (quantity_concept<Q1>
                          && quantity_concept<Q2>
                          && Q1::always_constexpr_unit
                          && Q2::always_constexpr_unit)
                static constexpr auto multiply(Q1 x, Q2 y) {
                    using r_repr_type = std::common_type_t<typename Q1::repr_type,
                                                           typename Q2::repr_type>;
                    using r_int_type = std::common_type_t<typename Q1::ratio_int_type,
                                                          typename Q2::ratio_int_type>;
                    using r_int2x_type = std::common_type_t<typename Q1::ratio_int2x_type,
                                                            typename Q2::ratio_int2x_type>;

                    constexpr auto rr = detail::su_product<r_int_type, r_int2x_type>(x.unit().natural_unit_,
                                                                                     y.unit().natural_unit_);

                    r_repr_type r_scale = (((rr.outer_scale_sq_ == 1.0)
                                            ? 1.0
                                            : ::sqrt(rr.outer_scale_sq_))
                                           * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                           * static_cast<r_repr_type>(x.scale())
                                           * static_cast<r_repr_type>(y.scale()));

                    return quantity<detail::su_promote<r_int_type>(rr.natural_unit_),
                                    r_repr_type>(r_scale);
                }

                template <typename Q1, typename Q2>
                requires (quantity_concept<Q1>
                          && quantity_concept<Q2>
                          && Q1::always_constexpr_unit
                          && Q2::always_constexpr_unit)
                static constexpr auto divide(Q1 x, Q2 y) {
                    using r_repr_type = std::common_type_t<typename Q1::repr_type,
                                                           typename Q2::repr_type>;
                    using r_int_type = std::common_type_t<typename Q1::ratio_int_type,
                                                          typename Q2::ratio_int_type>;
                    using r_int2x_type = std::common_type_t<typename Q1::ratio_int2x_type,
                                                            typename Q2::ratio_int2x_type>;

                    constexpr auto rr = detail::su_ratio<r_int_type, r_int2x_type>(x.unit().natural_unit_,
                                                                                   y.unit().natural_unit_);

                    r_repr_type r_scale = (((rr.outer_scale_sq_ == 1.0)
                                            ? 1.0
                                            : ::sqrt(rr.outer_scale_sq_))
                                           * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                           * static_cast<r_repr_type>(x.scale())
                                           / static_cast<r_repr_type>(y.scale()));

                    return quantity<detail::su_promote<r_int_type>(rr.natural_unit_),
                                    r_repr_type>(r_scale);
                }

                template <typename Q1, typename Q2>
                requires(quantity_concept<Q1>
                         && quantity_concept<Q2>
                         && Q1::always_constexpr_unit
                         && Q2::always_constexpr_unit)
                static constexpr auto add(Q1 x, Q2 y) {
                    using r_repr_type = std::common_type_t<typename Q1::repr_type,
                                                           typename Q2::repr_type>;
                    using r_int_type = std::common_type_t<typename Q1::ratio_int_type,
                                                          typename Q2::ratio_int_type>;
                    using r_int2x_type = std::common_type_t<typename Q1::ratio_int2x_type,
                                                            typename Q2::ratio_int2x_type>;
                    /* conversion to get y in same units as x: multiply by y/x */
                    auto rr = detail::su_ratio<r_int_type, r_int2x_type>(y.unit().natural_unit_,
                                                                         x.unit().natural_unit_);

                    if (rr.natural_unit_.is_dimensionless()) {
                        r_repr_type r_scale = (static_cast<r_repr_type>(x.scale())
                                               + (::sqrt(rr.outer_scale_sq_)
                                                  * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                                  * static_cast<r_repr_type>(y.scale())));

                        return quantity<x.s_scaled_unit, r_repr_type>(r_scale);
                    } else {
                        /* units don't match! */
                        return quantity<x.s_scaled_unit, r_repr_type>(std::numeric_limits<r_repr_type>::quiet_NaN());
                    }
                }

                template <typename Q1, typename Q2>
                requires(quantity_concept<Q1>
                         && quantity_concept<Q2>
                         && Q1::always_constexpr_unit
                         && Q2::always_constexpr_unit)
                static constexpr auto subtract(Q1 x, Q2 y) {
                    using r_repr_type = std::common_type_t<typename Q1::repr_type,
                                                           typename Q2::repr_type>;
                    using r_int_type = std::common_type_t<typename Q1::ratio_int_type,
                                                          typename Q2::ratio_int_type>;
                    using r_int2x_type = std::common_type_t<typename Q1::ratio_int2x_type,
                                                            typename Q2::ratio_int2x_type>;
                    /* conversion to get y in same units as x: multiply by y/x */
                    auto rr = detail::su_ratio<r_int_type, r_int2x_type>(y.unit(), x.unit());

                    if (rr.natural_unit_.is_dimensionless()) {
                        r_repr_type r_scale = (static_cast<r_repr_type>(x.scale())
                                               - (::sqrt(rr.outer_scale_sq_)
                                                  * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                                  * static_cast<r_repr_type>(y.scale())));

                        return quantity<x.s_unit, r_repr_type>(r_scale);
                    } else {
                        /* units don't match! */
                        return quantity<x.s_unit, r_repr_type>(std::numeric_limits<r_repr_type>::quiet_NaN());
                    }
                }
            };
        } /*namespace detail*/

        template <auto Unit, typename Q1>
        requires(quantity_concept<Q1>
                 && Q1::always_constexpr_unit)
        constexpr auto
        with_units(const Q1 & x) {
            return x.template rescale_ext<Unit>();
        }

        template <typename Q1, typename Q2, auto Unit = Q2::s_scaled_unit>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && Q1::always_constexpr_unit
                  && Q2::always_constexpr_unit)
        constexpr auto
        with_units_from(const Q1 & x, const Q2 & y)
        {
            return x.template rescale_ext<Unit>();
        }

        template <typename Repr2, typename Q1>
        requires (quantity_concept<Q1>
                  && Q1::always_constexpr_unit)
        constexpr auto
        with_repr(const Q1 & x)
        {
            return x.template with_repr<Repr2>();
        }

        /** note: won't have constexpr result w/ fractional dimension until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Q1, typename Q2>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && Q1::always_constexpr_unit
                  && Q2::always_constexpr_unit)
        constexpr auto
        operator* (const Q1 & x, const Q2 & y)
        {
            return detail::quantity_util::multiply(x, y);
        }

        /** note: won't have constexpr result w/ fractional dimension until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Q1, typename Q2>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && Q1::always_constexpr_unit
                  && Q2::always_constexpr_unit)
        constexpr auto
        operator/ (const Q1 & x, const Q2 & y)
        {
            return detail::quantity_util::divide(x, y);
        }

        /** note: won't have constexpr result w/ fractional dimension until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Q1, typename Q2>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && Q1::always_constexpr_unit
                  && Q2::always_constexpr_unit)
        constexpr auto
        operator+ (const Q1 & x, const Q2 & y)
        {
            return detail::quantity_util::add(x, y);
        }

        /** note: won't have constexpr result w/ fractional dimension until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Q1, typename Q2>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && Q1::always_constexpr_unit
                  && Q2::always_constexpr_unit)
        constexpr auto
        operator- (const Q1 & x, const Q2 & y)
        {
            return detail::quantity_util::subtract(x, y);
        }

        namespace qty {
            // ----- mass -----

            /** create quantity representing @p x picograms of mass, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto picograms(Repr x) { return quantity<u::picogram, Repr>(x); }

            /** create quantity representing @p x nanograms of mass, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto nanograms(Repr x) { return quantity<u::nanogram, Repr>(x); }

            /** create quantity representing @p x micrograms of mass, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto micrograms(Repr x) { return quantity<u::microgram, Repr>(x); }

            /** create quantity representing @p x milligrams of mass, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto milligrams(Repr x) { return quantity<u::milligram, Repr>(x); }

            /** create quantity representing @p x grams of mass, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto grams(Repr x) { return quantity<u::gram, Repr>(x); }

            /** create quantity representing @p x kilograms of mass,  with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto kilograms(Repr x) { return quantity<u::kilogram, Repr>(x); }

            /** create quantity representing @p x tonnes of mass, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto tonnes(Repr x) { return quantity<u::tonne, Repr>(x); }

            /** create quantity representing @p x kilotonnes of mass, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto kilotonnes(Repr x) { return quantity<u::kilotonne, Repr>(x); }

            /** create quantity representing @p x megatonnes of mass, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto megatonnes(Repr x) { return quantity<u::megatonne, Repr>(x); }

            /** create quantity representing @p x gigatonnes of mass, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto gigatonnes(Repr x) { return quantity<u::gigatonne, Repr>(x); }
        }

        namespace qty {
            // ----- mass constants ----

            /** @brief a quantity representing 1 picogram of mass, with compile-time unit representation **/
            static constexpr auto picogram = picograms(1);
            /** @brief a quantity representing 1 nanogram of mass, with compile-time unit representation **/
            static constexpr auto nanogram = nanograms(1);
            /** @brief a quantity representing 1 microgram of mass, with compile-time unit representation **/
            static constexpr auto microgram = micrograms(1);
            /** @brief a quantity representing 1 milligram of mass, with compile-time unit representation **/
            static constexpr auto milligram = milligrams(1);
            /** @brief a quantity representing 1 gram of mass, with compile-time unit representation **/
            static constexpr auto gram = grams(1);
            /** @brief a quantity representing 1 kilogram of mass, with compile-time unit representation **/
            static constexpr auto kilogram = kilograms(1);
            /** @brief a quantity representing 1 metric tonne of mass, with compile-time unit representation **/
            static constexpr auto tonne = tonnes(1);
            /** @brief a quantity representing 1 metric kilotonne of mass, with compile-time unit representation **/
            static constexpr auto kilotonne = kilotonnes(1);
        } /*namespace qty*/

        namespace qty {
            // ----- distance -----

            /** create quantity representing @p x picometers of distance, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto picometers(Repr x) { return quantity<u::picometer, Repr>(x); }

            /** create quantity representing @p x nanometers of distance, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto nanometers(Repr x) { return quantity<u::nanometer, Repr>(x); }

            /** create quantity representing @p x micrometers of distance, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto micrometers(Repr x) { return quantity<u::micrometer, Repr>(x); }

            /** create quantity representing @p x millimeters of distance, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto millimeters(Repr x) { return quantity<u::millimeter, Repr>(x); }

            /** create quantity representing @p x meters of distance, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto meters(Repr x) { return quantity<u::meter, Repr>(x); }

            /** create quantity representing @p x kilometers of distance, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto kilometers(Repr x) { return quantity<u::kilometer, Repr>(x); }

            /** create quantity representing @p x megameters of distance, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto megameters(Repr x) { return quantity<u::megameter, Repr>(x); }

            /** create quantity representing @p x gigameters of distance,
             *  with compile-time unit operations
             **/
            template <typename Repr>
            inline constexpr auto gigameters(Repr x) { return quantity<u::gigameter, Repr>(x); }

            /** create quantity representing @p x light-seconds of distance,
             *  with compile-time unit operations.
             **/
            template <typename Repr>
            inline constexpr auto lightseconds(Repr x) { return quantity<u::lightsecond, Repr>(x); }

            /** create quantity representing @p x astronomical units of distance,
             *  with compile-time unit representation
             **/
            template <typename Repr>
            inline constexpr auto astronomicalunits(Repr x) { return quantity<u::astronomicalunit, Repr>(x); }

            /** create quantity representing @p x inches of distance, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto inches(Repr x) { return quantity<u::inch, Repr>(x); }
            /** create quantity representing @p x feet of distance, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto feet(Repr x) { return quantity<u::foot, Repr>(x); }
            /** create quantity representing @p x yards of distance, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto yards(Repr x) { return quantity<u::yard, Repr>(x); }
            /** create quantity representing @p x statute miles of distance, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto miles(Repr x) { return quantity<u::mile, Repr>(x); }
        }

        namespace qty {
            // ----- distance constants -----

            /** a quantity representing 1 picometer of distance, with compile-time unit representation **/
            static constexpr auto picometer = picometers(1);
            /** a quantity representing 1 nanometer of distance, with compile-time unit representation **/
            static constexpr auto nanometer = nanometers(1);
            /** a quantity representing 1 micrometer of distance, with compile-time unit representation **/
            static constexpr auto micrometer = micrometers(1);
            /** @brief a quantity representing 1 millimeter of distance, with compile-time unit representation **/
            static constexpr auto millimeter = millimeters(1);
            /** @brief a quantity representing 1 meter of distance, with compile-time unit representation **/
            static constexpr auto meter = meters(1);
            /** @brief a quantity representing 1 kilometer of distance, with compile-time unit representation **/
            static constexpr auto kilometer = kilometers(1);
            /** @brief a quantity representing 1 megameter of distance, with compile-time unit representation **/
            static constexpr auto megameter = megameters(1);
            /** @brief a quantity representing 1 gigameter of distance, with compile-time unit representation **/
            static constexpr auto gigameter = gigameters(1);

            /** @brief a quantity representing exactly 1 lightsecond of distance,  with compile-time unit representation **/
            static constexpr auto lightsecond = lightseconds(1);
            /** @brief a quantity representing exactly 1 astronomical unit of distance,  with compile-time unit representation **/
            static constexpr auto astronomicalunit = astronomicalunits(1);

        } /*namespace qty*/

        namespace qty {
            // ----- time -----

            /** create quantity representing @p x picoseconds of time, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto picoseconds(Repr x) { return quantity<u::picosecond, Repr>(x); }

            /** create quantity representing @p x nanoseconds of time, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto nanoseconds(Repr x) { return quantity<u::nanosecond, Repr>(x); }

            /** create quantity representing @p x microseconds of time, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto microseconds(Repr x) { return quantity<u::microsecond, Repr>(x); }

            /** create quantity representing @p x milliseconds of time, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto milliseconds(Repr x) { return quantity<u::millisecond, Repr>(x); }

            /** create quantity representing @p x seconds of time, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto seconds(Repr x) { return quantity<u::second, Repr>(x); }

            /** create quantity representing @p x minutes of time, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto minutes(Repr x) { return quantity<u::minute, Repr>(x); }

            /** create quantity representing @p x hours of time, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto hours(Repr x) { return quantity<u::hour, Repr>(x); }

            /** create quantity representing @p x exactly-24-hour days of time, with compile-time unit operations **/
            template <typename Repr>
            inline constexpr auto days(Repr x) { return quantity<u::day, Repr>(x); }

            /** creeate quantity representing @p x weeks of time,
             *  with compile-time unit operations.  Each week has exactly 7 24-hour days.
             **/
            template <typename Repr>
            inline constexpr auto weeks(Repr x) { return quantity<u::week, Repr>(x); }

            /** create quantity representing @p x months of time,
             *  with compile-time unit operations.  Each month has exactly 30 24-hour days
             **/
            template <typename Repr>
            inline constexpr auto months(Repr x) { return quantity<u::month, Repr>(x); }

            /** create quantity representing @p x years of time,
             *  with compile-time unit operations.  Each year has exactly 365.25 24-hour days
             **/
            template <typename Repr>
            inline constexpr auto years(Repr x) { return quantity<u::year, Repr>(x); }

            /** create quantity representing @p x '250-day years' of time.
             *  250 represents approximate number of business days in a calendar year.
             **/
            template <typename Repr>
            inline constexpr auto year250s(Repr x) { return quantity<u::year250, Repr>(x); }

            /** create quantity representing @p x '360-day years' of time **/
            template <typename Repr>
            inline constexpr auto year360s(Repr x) { return quantity<u::year360, Repr>(x); }

            /** create quantity representing @p x '365-day years' of time **/
            template <typename Repr>
            inline constexpr auto year365s(Repr x) { return quantity<u::year365, Repr>(x); }
        }

        namespace qty {
            // ----- time constants ----

            /** @brief a quantity representing 1 second of time, with compile-time unit representation **/
            static constexpr auto second = seconds(1);

            /** @brief a quantity representing 1 minute of time, with compile-time unit representation **/
            static constexpr auto minute = minutes(1);

            /** @brief a quantity representing 1 hour of time, with compile-time unit representation **/
            static constexpr auto hour = hours(1);

            /** @brief a quantity representing 1 day of time (exactly 24 hours), with compile-time unit representation **/
            static constexpr auto day = days(1);
        } /*namespace qty*/

        namespace qty {
            // ----- volatility -----

            /* variance expressed has dimension 1/t;
             * volatility ~ sqrt(variance),  has dimension 1/sqrt(t)
             */

            /** create quantity representing @p x units of 30-day volatility, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto volatility_30d(Repr x) { return quantity<u::volatility_30d, Repr>(x); }

            /** create quantity representing @p x units of 250-day volatility, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto volatility_250d(Repr x) { return quantity<u::volatility_250d, Repr>(x); }

            /** create quantity representing @p x units of 360-day volatility, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto volatility_360d(Repr x) { return quantity<u::volatility_360d, Repr>(x); }

            /** create quantity representing @p x units of 365-day volatility, with compile-time unit representation **/
            template <typename Repr>
            inline constexpr auto volatility_365d(Repr x) { return quantity<u::volatility_365d, Repr>(x); }
        } /*namespace qty*/

        /* reminder: see [quantity_ops.hpp] for operator* etc */
    } /*namespace qty*/
} /*namespace xo*/

/** end quantity.hpp **/
