/* @file quantity.hpp */

#pragma once

#include "quantity_concept.hpp"
#include "unit.hpp"
//#include "xo/reflect/Reflect.hpp"
//#include "xo/indentlog/scope.hpp"

namespace xo {
    namespace unit {
        /** @class promoter
         *
         *  Aux class assister for quantity::promote()
         **/
        template <typename Unit, typename Repr, bool Dimensionless = dimensionless_v<Unit> >
        struct promoter;

        // ----- quantity -----

        /** @class quantity
         *
         *  @brief represets a scalar quantity;  enforces dimensional consistency at compile time
         *
         *  Repr representation.
         *  Unit unit
         *  Assert use to specify required unit dimension
         *
         *  Require:
         *  - Repr copyable, assignable
         *  - Repr = 0
         *  - Repr = 1
         *  - Repr + Repr -> Repr
         *  - Repr - Repr -> Repr
         *  - Repr * Repr -> Repr
         *  - Repr / Repr -> Repr
         **/
        template <typename Unit, typename Repr = double>
        class quantity {
        public:
            using unit_type = Unit;
            using repr_type = Repr;

            /* non-unity compile-time scale factors can arise during unit conversion;
             * for example see method quantity::in_units_of()
             */
            static_assert(std::same_as< typename Unit::scalefactor_type, std::ratio<1> >);
            static_assert(std::same_as< typename Unit::canon_type, typename Unit::canon_type >);

        public:
            constexpr quantity() = default;
            constexpr quantity(quantity const & x) = default;
            constexpr quantity(quantity && x) = default;

            template <dim BasisDim>
            using find_bpu_t = unit_find_bpu_t<unit_type, BasisDim>;

            /**
             *  For example:
             *    auto q = qty::milliseconds(5) * qty::seconds(1);
             *  then
             *    q.basis_power<dim::time> -> 2
             *    q.basis_power<dim::mass> -> 0
             **/
            template <dim BasisDim, typename PowerRepr = int>
            static constexpr PowerRepr basis_power = from_ratio<PowerRepr, typename find_bpu_t<BasisDim>::power_type>();

            /** @brief get scale value (relative to unit) (@ref scale_) **/
            constexpr Repr scale() const { return scale_; }
            /** @brief abbreviation for this quantity's units **/
            static constexpr char const * unit_cstr() { return unit_abbrev_v<unit_type>.c_str(); }
            /** @brief return unit quantity -- amount with this Unit that has representation = 1 **/
            static constexpr quantity unit_quantity() { return quantity(1); }
            /** @brief promote representation to quantity.  Same as multiplying by Unit **/
            static constexpr auto promote(Repr x) {
                //std::cerr << "quantity<U,R>::promote: x=" << x << ", R=" << reflect::Reflect::require<Repr>()->canonical_name() << std::endl;
                return promoter<Unit, Repr>::promote(x);
            }

            template <typename Unit2>
            constexpr quantity<Unit2, Repr> with_unit() const { return *this; }

            template <typename Repr2>
            constexpr quantity<unit_type, Repr2> with_repr() const { return quantity<unit_type, Repr2>::promote(scale_); }

            template <typename Quantity2>
            auto multiply(Quantity2 y) const {
                //constexpr bool c_debug_flag = false;
                //using Reflect = xo::reflect::Reflect;

                //scope log(XO_DEBUG(c_debug_flag));

                static_assert(quantity_concept<Quantity2>);

                /* unit: may have non-unit scalefactor_type */
                using unit_product_type = unit_cartesian_product<Unit, typename Quantity2::unit_type>;
                using exact_unit_type = unit_product_type::exact_unit_type;
                using norm_unit_type = normalize_unit_t<exact_unit_type>;

                using exact_scalefactor_type = exact_unit_type::scalefactor_type;
                constexpr double c_scalefactor_inexact = unit_product_type::c_scalefactor_inexact;

                using repr_type = std::common_type_t<repr_type, typename Quantity2::repr_type>;

                repr_type r_scale = ((scale() * y.scale() * c_scalefactor_inexact * exact_scalefactor_type::num)
                                     / exact_scalefactor_type::den);

#              ifdef NOT_USING_DEBUG
                log && log(xtag("unit_product_type", Reflect::require<unit_product_type>()->canonical_name()));
                log && log(xtag("exact_unit_type", Reflect::require<exact_unit_type>()->canonical_name()));
                log && log(xtag("norm_unit_type", Reflect::require<norm_unit_type>()->canonical_name()));
                log && log(xtag("exact_scalefactor_type", Reflect::require<exact_scalefactor_type>()->canonical_name()));
                log && log(xtag("c_scalefactor_inexact", c_scalefactor_inexact));
                log && log(xtag("repr_type", Reflect::require<repr_type>()->canonical_name()));
                log && log(xtag("repr_type", Reflect::require<repr_type>()->canonical_name()));
#              endif

                return quantity<norm_unit_type, repr_type>::promote(r_scale);
            }

            template <typename Quantity2>
            auto divide(Quantity2 y) const {
                using unit_divide_type = unit_divide<Unit, typename Quantity2::unit_type>;
                using exact_unit_type = unit_divide_type::exact_unit_type;
                using norm_unit_type = normalize_unit_t<exact_unit_type>;

                using exact_scalefactor_type = exact_unit_type::scalefactor_type;
                constexpr double c_scalefactor_inexact = unit_divide_type::c_scalefactor_inexact;

                using repr_type = std::common_type_t<repr_type, typename Quantity2::repr_type>;

                repr_type r_scale = ((scale() * c_scalefactor_inexact * exact_scalefactor_type::num)
                                     / (y.scale() * exact_scalefactor_type::den));

#              ifdef NOT_USING_DEBUG
                using xo::reflect::Reflect;
                scope log(XO_DEBUG(true /*c_debug_flag*/));
                log && log(xtag("unit_divide_type", Reflect::require<unit_divide_type>()->canonical_name()));
                log && log(xtag("exact_unit_type", Reflect::require<exact_unit_type>()->canonical_name()));
                log && log(xtag("norm_unit_type", Reflect::require<norm_unit_type>()->canonical_name()));
                log && log(xtag("exact_scalefactor_type", Reflect::require<exact_scalefactor_type>()->canonical_name()));
                log && log(xtag("c_scalefactor_inexact", c_scalefactor_inexact));
                log && log(xtag("r_scale", r_scale));
                log && log(xtag("repr_type", Reflect::require<repr_type>()->canonical_name()));
#              endif

                return quantity<norm_unit_type, repr_type>::promote(r_scale);
            }

            // quantity operator*=()
            // quantity operator/=()

            /**
             *  scale by dimensionless number
             **/
            template <typename Repr2>
            auto scale_by(Repr2 x) const {
                static_assert(!quantity_concept<Repr2>);

                using r_repr_type = std::common_type_t<repr_type, Repr2>;

                r_repr_type r_scale = this->scale_ * x;

                //std::cerr << "quantity::scale_by: scale=" << scale << ", repr_type=" << reflect::Reflect::require<repr_type>()->canonical_name() << std::endl;

                return quantity<unit_type, r_repr_type>::promote(r_scale);
            }

            /**
             *  divide by dimensionless number
             **/
            template <typename Repr2>
            auto divide_by(Repr2 x) const {
                using r_repr_type = std::common_type_t<repr_type, Repr2>;

                r_repr_type r_scale = this->scale_ / x;

                return quantity<unit_type, r_repr_type>::promote(r_scale);
            }

            /**
             *  divide dimensionless number by this quantity
             **/
            template <typename Repr2>
            auto divide_into(Repr2 x) const {
                using r_unit_type = unit_invert_t<Unit>;
                using r_repr_type = std::common_type_t<repr_type, Repr2>;

                r_repr_type r_scale = ((x * r_unit_type::scalefactor_type::num)
                                       / (this->scale_ * r_unit_type::scalefactor_type::den));

                return quantity<r_unit_type, r_repr_type>::promote(r_scale);
            }

            template <typename Unit2, typename Repr2>
            Repr2 in_units_of() const {
                // static_assert(dimension_of<Unit> == dimension_of<Unit2>);  // discard all the scaling values

                static_assert(same_dimension_v<Unit, Unit2>);

                using _convert_to_u2_type = unit_cartesian_product<Unit, unit_invert_t<Unit2>>;

                using exact_scalefactor_type = _convert_to_u2_type::exact_unit_type::scalefactor_type;
                constexpr double c_scalefactor_inexact = _convert_to_u2_type::c_scalefactor_inexact;

                // _convert_u2_type
                //  - scalefactor_type
                //  - dim_type
                //  - canon_type

                /* if _convert_u2_type isn't dimensionless,  then {Unit2, Unit} have different dimensions */

                return ((this->scale_ * c_scalefactor_inexact * exact_scalefactor_type::num) / exact_scalefactor_type::den);
            }

            template <typename Quantity2>
            quantity operator+=(Quantity2 y) {
                static_assert(std::same_as<
                              typename unit_type::canon_type,
                              typename Quantity2::unit_type::canon_type >);

                /* relying on assignment that correctly converts-to-lhs-units */
                quantity y2 = y;

                this->scale_ += y2.scale();

                return *this;
            }

            template <typename Quantity2>
            quantity operator-=(Quantity2 y) {
                static_assert(std::same_as<
                              typename unit_type::canon_type,
                              typename Quantity2::unit_type::canon_type >);

                quantity y2 = y;

                /* relying on assignment that correctly converts-to-lhs-units */
                this->scale_ -= y2.scale();

                return *this;
            }

            /* convert to quantity with same dimension, different {unit_type, repr_type} */
            template <typename Quantity2>
            operator Quantity2 () const {
                /* avoid truncating precision when converting:
                 * use best available representation
                 */
                using tmp_repr_type = std::common_type_t<repr_type, typename Quantity2::repr_type>;

                return Quantity2::promote(this->in_units_of<typename Quantity2::unit_type, tmp_repr_type>());
            }

            void display(std::ostream & os) const {
                os << this->scale() << unit_cstr();
            }

            quantity & operator=(quantity const & x) = default;
            quantity & operator=(quantity && x) = default;

        private:
            explicit constexpr quantity(Repr x) : scale_{x} {}

            friend class promoter<Unit, Repr, true>;
            friend class promoter<Unit, Repr, false>;

        private:
            /** @brief quantity represents this multiple of a unit (that has compile-time outer-scalefactor of 1) **/
            Repr scale_ = 0;
        }; /*quantity*/

        // ----- promoter -----

        /* collapse dimensionless quantity to its repr_type> */
        template <typename Unit, typename Repr>
        struct promoter<Unit, Repr, /*Dimensionless*/ true> {
            static constexpr Repr promote(Repr x) { return x; };
        };

        template <typename Unit, typename Repr>
        struct promoter<Unit, Repr, /*Dimensionless*/ false> {
            static constexpr quantity<Unit, Repr> promote(Repr x) { return quantity<Unit, Repr>(x); }
        };

        // ----- operator+ -----

        template <typename Quantity1, typename Quantity2>
        inline Quantity1 operator+ (Quantity1 x, Quantity2 y) {
            static_assert(same_dimension_v<typename Quantity1::unit_type, typename Quantity2::unit_type>);

            /* convert y to match units used by x;
             * would fail at compile time if this isn't well-defined
             */
            Quantity1 y2 = y;

            return Quantity1::promote(x.scale() + y2.scale());
        }

        template <typename Quantity1, typename Quantity2>
        inline Quantity1 operator- (Quantity1 x, Quantity2 y) {
            static_assert(std::same_as
                          < typename Quantity1::unit_type::dimension_type::canon_type,
                          typename Quantity2::unit_type::dimension_type::canon_type >);

            /* convert y to match units used by x */
            Quantity1 y2 = y;

            return Quantity1::promote(x.scale() - y2.scale());
        }

        template <typename Quantity>
        inline Quantity operator- (Quantity x) {
            return Quantity::promote(- x.scale());
        }

        template <typename Quantity1, typename Quantity2>
        inline auto operator* (Quantity1 x, Quantity2 y) {
            static_assert(quantity_concept<Quantity1>);
            static_assert(quantity_concept<Quantity2>);

            return x.multiply(y);
        }

        /** e.g. DECLARE_LH_MULT(int32_t) **/
#      define DECLARE_LH_MULT(lhtype)                  \
        template <typename Quantity>                   \
        inline auto                                    \
        operator* (lhtype x, Quantity y) {             \
            static_assert(quantity_concept<Quantity>); \
            return y.scale_by(x);                      \
        }

        DECLARE_LH_MULT(int8_t);
        DECLARE_LH_MULT(uint8_t);
        DECLARE_LH_MULT(int16_t);
        DECLARE_LH_MULT(uint16_t);
        DECLARE_LH_MULT(int32_t);
        DECLARE_LH_MULT(uint32_t);
        DECLARE_LH_MULT(int64_t);
        DECLARE_LH_MULT(uint64_t);
        DECLARE_LH_MULT(float);
        DECLARE_LH_MULT(double);
#      undef DECLARE_LH_MULT

        /** e.g. DECLARE_RH_MULT(int32_t) **/
#      define DECLARE_RH_MULT(rhtype)                  \
        template <typename Quantity>                   \
        inline auto                                    \
        operator* (Quantity x, rhtype y) {             \
            static_assert(quantity_concept<Quantity>); \
            return x.scale_by(y);                      \
        }

        DECLARE_RH_MULT(int8_t);
        DECLARE_RH_MULT(uint8_t);
        DECLARE_RH_MULT(int16_t);
        DECLARE_RH_MULT(uint16_t);
        DECLARE_RH_MULT(int32_t);
        DECLARE_RH_MULT(uint32_t);
        DECLARE_RH_MULT(int64_t);
        DECLARE_RH_MULT(uint64_t);
        DECLARE_RH_MULT(float);
        DECLARE_RH_MULT(double);
#      undef DECLARE_LH_MULT

        template <typename Quantity1, typename Quantity2>
        inline auto operator/ (Quantity1 x, Quantity2 y) {
            static_assert(quantity_concept<Quantity1>);
            static_assert(quantity_concept<Quantity2>);

            return x.divide(y);
        }

#      define DECLARE_LH_DIV(lhtype)                   \
        template <typename Quantity>                   \
        inline auto                                    \
        operator/ (lhtype x, Quantity y) {             \
            static_assert(quantity_concept<Quantity>); \
            return y.divide_into(x);                   \
        }

        DECLARE_LH_DIV(int8_t);
        DECLARE_LH_DIV(uint8_t);
        DECLARE_LH_DIV(int16_t);
        DECLARE_LH_DIV(uint16_t);
        DECLARE_LH_DIV(int32_t);
        DECLARE_LH_DIV(uint32_t);
        DECLARE_LH_DIV(int64_t);
        DECLARE_LH_DIV(uint64_t);
        DECLARE_LH_DIV(float);
        DECLARE_LH_DIV(double);
#      undef DECLARE_LH_DIV

#      define DECLARE_RH_DIV(rhtype) \
        template <typename Quantity> \
        inline auto \
        operator/ (Quantity x, rhtype y) { \
            static_assert(quantity_concept<Quantity>); \
            return x.divide_by(y); \
        }

        DECLARE_RH_DIV(int8_t)
        DECLARE_RH_DIV(uint8_t)
        DECLARE_RH_DIV(int16_t)
        DECLARE_RH_DIV(uint16_t)
        DECLARE_RH_DIV(int32_t)
        DECLARE_RH_DIV(uint32_t)
        DECLARE_RH_DIV(int64_t)
        DECLARE_RH_DIV(uint64_t)
        DECLARE_RH_DIV(float)
        DECLARE_RH_DIV(double)
#      undef DECLARE_RH_DIV

        template <typename Unit, typename Repr>
        inline std::ostream &
        operator<< (std::ostream & os, quantity<Unit, Repr> const & x) {
            x.display(os);
            return os;
        }

        namespace qty {
            // ----- mass -----

            template <typename Repr = double>
            inline auto milligrams(Repr x) -> quantity<units::milligram, Repr> {
                return quantity<units::milligram, Repr>::promote(x);
            };

            template <typename Repr = double>
            inline auto grams(Repr x) -> quantity<units::gram, Repr> {
                return quantity<units::gram, Repr>::promote(x);
            };

            template <typename Repr = double>
            inline auto kilograms(Repr x) -> quantity<units::kilogram, Repr> {
                return quantity<units::kilogram, Repr>::promote(x);
            };

            // ----- distance -----

            template <typename Repr = double>
            inline auto millimeters(Repr x) -> quantity<units::millimeter, Repr> {
                return quantity<units::millimeter, Repr>::promote(x);
            }

            template <typename Repr = double>
            inline auto meters(Repr x) -> quantity<units::meter, Repr> {
                return quantity<units::meter, Repr>::promote(x);
            }

            template <typename Repr = double>
            inline auto kilometers(Repr x) -> quantity<units::kilometer, Repr> {
                return quantity<units::kilometer, Repr>::promote(x);
            }

            // ----- time -----

            template <typename Repr = double>
            inline auto nanoseconds(Repr x) -> quantity<units::nanosecond, Repr> {
                return quantity<units::nanosecond, Repr>::promote(x);
            }

            template <typename Repr = double>
            inline auto microseconds(Repr x) -> quantity<units::microsecond, Repr> {
                return quantity<units::microsecond, Repr>::promote(x);
            }

            template <typename Repr = double>
            inline auto milliseconds(Repr x) -> quantity<units::millisecond, Repr> {
                return quantity<units::millisecond, Repr>::promote(x);
            }

            template <typename Repr = double>
            inline auto seconds(Repr x) -> quantity<units::second, Repr> {
                return quantity<units::second, Repr>::promote(x);
            }

            template <typename Repr = double>
            inline auto minutes(Repr x) -> quantity<units::minute, Repr> {
                return quantity<units::minute, Repr>::promote(x);
            }

            template <typename Repr = double>
            inline auto hours(Repr x) -> quantity<units::hour, Repr> {
                return quantity<units::hour, Repr>::promote(x);
            }

            template <typename Repr = double>
            inline auto days(Repr x) -> quantity<units::day, Repr> {
                return quantity<units::day, Repr>::promote(x);
            }

            // ----- time/volatility -----

            /** quantity in units of 1/sqrt(1dy) **/
            template <typename Repr = double>
            inline auto volatility1d(Repr x) -> quantity<units::volatility_1d, Repr> {
                return quantity<units::volatility_1d, Repr>::promote(x);
            }

            /** quantity in units of 1/sqrt(30days)
             **/
            template <typename Repr = double>
            inline auto volatility30d(Repr x) -> quantity<units::volatility_30d, Repr> {
                return quantity<units::volatility_30d, Repr>::promote(x);
            }

            /** quantity in units of 1/sqrt(250days)
             **/
            template <typename Repr = double>
            inline auto volatility250d(Repr x) -> quantity<units::volatility_250d, Repr> {
                return quantity<units::volatility_250d, Repr>::promote(x);
            }
        } /*namespace qty*/

        namespace unit_qty {
            /** @brief quantity with mass dimension, representing 1mg (1 milligram = 10^-3 grams) **/
            static constexpr auto milligram = qty::milligrams(1.0);
            /** @brief quantity with mass dimension, representing 1g (1 gram) **/
            static constexpr auto gram = qty::grams(1.0);
            /** @brief quantity with mass dimension, representing 1kg (1 kilogram = 1000 grams) **/
            static constexpr auto kilogram = qty::kilograms(1.0);

            /** @brief quantity with length dimension representing 1mm (10^-3 meters) **/
            static constexpr auto millimeter = qty::millimeters(1.0);
            /** @brief quantity with length dimension representing 1m (1 meter) **/
            static constexpr auto meter = qty::meters(1.0);
            /** @brief quantity with length dimension representing 1km (1 kilometer = 1000 meters) **/
            static constexpr auto kilometer = qty::kilometers(1.0);

            /** @brief quantity with time dimension representing 1ns (1 nanosecond = 10^-9 seconds) **/
            static constexpr auto nanosecond = qty::microseconds(1);
            /** @brief quantity with time dimension representing 1us (1 microsecond = 10^-6 seconds) **/
            static constexpr auto microsecond = qty::microseconds(1);
            /** @brief quantity with time dimension representing 1ms (1 milliseconds = 10^-3 seconds) **/
            static constexpr auto millisecond = qty::milliseconds(1);
            /** @brief quantity with time dimension representing 1s (1 second) **/
            static constexpr auto second = qty::seconds(1);
            /** @brief quantity with time dimension representing 1min (1 minute = 60 seconds) **/
            static constexpr auto minute = qty::minutes(1);
            /** @brief quantity with time dimension representing 1hr (1 hour = 60 minutes) **/
            static constexpr auto hour = qty::hours(1);
            /** @brief quantity with time dimension representing 1dy (1 day = 24 hours) **/
            static constexpr auto day = qty::days(1);
        }

    } /*namespace unit*/
} /*namespace xo*/

/* end quantity.hpp */
