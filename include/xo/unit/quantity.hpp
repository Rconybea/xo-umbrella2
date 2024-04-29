/** @file quantity.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "natural_unit.hpp"
#include "scaled_unit.hpp"

namespace xo {
    namespace qty {
        /** @class quantity
         *  @brief represent a scalar quantity with associated units.
         *
         *  Enforce dimensional consistency at compile time.
         *  sizeof(quantity) == sizeof(Repr).
         **/
        template <
            typename Repr = double,
            typename Int = std::int64_t,
            natural_unit<Int> NaturalUnit = natural_unit<Int>(),
            typename Int2x = detail::width2x<Int>
            >
        class quantity {
        public:
            using repr_type = Repr;
            using unit_type = natural_unit<Int>;
            using ratio_int_type = Int;
            using ratio_int2x_type = Int2x;

        public:
            constexpr quantity() : scale_{0} {}
            explicit constexpr quantity(Repr scale) : scale_{scale} {}

            constexpr const repr_type & scale() const { return scale_; }
            constexpr const unit_type & unit() const { return s_unit; }

            constexpr nu_abbrev_type abbrev() const { return s_unit.abbrev(); }

        public: /* need public members so that a quantity instance can be a non-type template parameter (is a structural type) */
            static constexpr natural_unit<Int> s_unit = NaturalUnit;

            Repr scale_ = Repr{};
        };

        namespace qty {
            // ----- mass -----

            inline constexpr auto picograms(double x) { return quantity<double, std::int64_t, nu::picogram>(x); }
            inline constexpr auto nanograms(double x) { return quantity<double, std::int64_t, nu::nanogram>(x); }
            inline constexpr auto micrograms(double x) { return quantity<double, std::int64_t, nu::microgram>(x); }
            inline constexpr auto milligrams(double x) { return quantity<double, std::int64_t, nu::milligram>(x); }
            inline constexpr auto grams(double x) { return quantity<double, std::int64_t, nu::gram>(x); }
            inline constexpr auto kilograms(double x) { return quantity<double, std::int64_t, nu::kilogram>(x); }
            inline constexpr auto tonnes(double x) { return quantity<double, std::int64_t, nu::tonne>(x); }
            inline constexpr auto kilotonnes(double x) { return quantity<double, std::int64_t, nu::kilotonne>(x); }
            inline constexpr auto megatonnes(double x) { return quantity<double, std::int64_t, nu::megatonne>(x); }
            inline constexpr auto gigatonnes(double x) { return quantity<double, std::int64_t, nu::gigatonne>(x); }

            // ----- distance -----

            inline constexpr auto picometers(double x) { return quantity<double, std::int64_t, nu::picometer>(x); }
            inline constexpr auto nanometers(double x) { return quantity<double, std::int64_t, nu::nanometer>(x); }
            inline constexpr auto micrometers(double x) { return quantity<double, std::int64_t, nu::micrometer>(x); }
            inline constexpr auto millimeters(double x) { return quantity<double, std::int64_t, nu::millimeter>(x); }
            inline constexpr auto meters(double x) { return quantity<double, std::int64_t, nu::meter>(x); }
            inline constexpr auto kilometers(double x) { return quantity<double, std::int64_t, nu::kilometer>(x); }
            inline constexpr auto megameters(double x) { return quantity<double, std::int64_t, nu::megameter>(x); }
            inline constexpr auto gigameters(double x) { return quantity<double, std::int64_t, nu::gigameter>(x); }

            inline constexpr auto lightseconds(double x) { return quantity<double, std::int64_t, nu::lightsecond>(x); }
            inline constexpr auto astronomicalunits(double x) { return quantity<double, std::int64_t, nu::astronomicalunit>(x); }

            // ----- time -----

            inline constexpr auto picoseconds(double x) { return quantity<double, std::int64_t, nu::picosecond>(x); }
            inline constexpr auto nanoseconds(double x) { return quantity<double, std::int64_t, nu::nanosecond>(x); }
            inline constexpr auto microseconds(double x) { return quantity<double, std::int64_t, nu::microsecond>(x); }
            inline constexpr auto milliseconds(double x) { return quantity<double, std::int64_t, nu::millisecond>(x); }
            inline constexpr auto seconds(double x) { return quantity<double, std::int64_t, nu::second>(x); }
            inline constexpr auto minutes(double x) { return quantity<double, std::int64_t, nu::minute>(x); }
            inline constexpr auto hours(double x) { return quantity<double, std::int64_t, nu::hour>(x); }
            inline constexpr auto days(double x) { return quantity<double, std::int64_t, nu::day>(x); }
            inline constexpr auto weeks(double x) { return quantity<double, std::int64_t, nu::week>(x); }
            inline constexpr auto months(double x) { return quantity<double, std::int64_t, nu::month>(x); }
            inline constexpr auto years(double x) { return quantity<double, std::int64_t, nu::year>(x); }
            inline constexpr auto year250s(double x) { return quantity<double, std::int64_t, nu::year250>(x); }
            inline constexpr auto year360s(double x) { return quantity<double, std::int64_t, nu::year360>(x); }
            inline constexpr auto year365s(double x) { return quantity<double, std::int64_t, nu::year365>(x); }
            //inline constexpr auto year366s(double x) { return quantity<double, std::int64_t, nu::year366>(x); }
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end quantity.hpp **/
