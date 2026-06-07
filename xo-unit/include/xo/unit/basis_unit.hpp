/** @file basis_unit.hpp **/

#pragma once

#include "native_unit.hpp"
#include "dimension.hpp"
//#include "basis_unit_abbrev.hpp"
#include "xo/ratio/ratio.hpp"

namespace xo {
    namespace qty {
        using bu_abbrev_type = flatstring<16>;
        using scalefactor_ratio_type = xo::ratio::ratio<std::int64_t>;
        using scalefactor2x_ratio_type = xo::ratio::ratio<__int128>;

        /** @class basis_unit
         *  @brief A dimensionless multiple of a single natively-specified basis dimension
         *
         *  For example "3600 minutes" or "1e-6 grams".
         *
         *  Members are public so that a @c basis_unit instance qualifies as a 'structural type',
         *  and therefore may be used as a non-type template parameter.
         **/
        struct basis_unit {
        public:
            /** @defgroup basis-unit-constructors basis_unit constructors **/
            ///@{
            /** sentinel basis unit: invalid dimension and zero scalefactor **/
            constexpr basis_unit() = default;
            /** basis unit representing multiple @p scalefactor of native dimension @p **/
            constexpr basis_unit(dimension native_dim,
                                 const scalefactor_ratio_type & scalefactor)
                : native_dim_{native_dim},
                  scalefactor_{scalefactor}
                {}
            ///@}

            /** @defgroup basis-unit-access-methods basis_unit access methods **/
            ///@{
            /** get @c native_dim member **/
            constexpr dimension native_dim() const { return native_dim_; }
            /** get @c scalefactor member **/
            constexpr const scalefactor_ratio_type & scalefactor() const { return scalefactor_; }
            ///@}

        public: /* public so instance can be a non-type template parameter (a structural type) */
            /** @defgroup basis-unit-instance-vars basis_unit instance variables **/
            ///@{
            /** @brief identifies a native unit, e.g. time **/
            dimension native_dim_ = dimension::invalid;
            /** @brief this unit defined as multiple scalefactor times native unit **/
            scalefactor_ratio_type scalefactor_ = {};
            ///@}
        };

        /** @defgroup basis-unit-comparison-support basis_unit comparisons **/
        ///@{
        /** @c true iff basis units are equal;
         *  both native dimension and scalefactor must be equal
         **/
        inline constexpr bool
        operator==(const basis_unit & x, const basis_unit & y)
        {
            return ((x.native_dim_ == y.native_dim_)
                    && (x.scalefactor_ == y.scalefactor_));
        }

        /** @c true iff bass units are not equal **/
        inline constexpr bool
        operator!=(const basis_unit & x, const basis_unit & y)
        {
            return ((x.native_dim_ != y.native_dim_)
                    || (x.scalefactor_ != y.scalefactor_));
        }
        ///@}

        namespace detail {
            /** @brief namespace for basis-unit constants and helpers **/
            namespace bu {
                // ----- mass -----

                constexpr basis_unit mass_unit(std::int64_t num, std::int64_t den) {
                    return basis_unit(dimension::mass, scalefactor_ratio_type(num, den));
                }

                /** @defgroup basis-unit-mass-units basis_unit mass units **/
                ///@{
                /** basis unit of 10^-12 grams **/
                constexpr basis_unit picogram         = mass_unit(               1, 1000000000000);
                /** basis unit of 10^-9 grams **/
                constexpr basis_unit nanogram         = mass_unit(               1,    1000000000);
                /** basis unit of 10^-6 grams **/
                constexpr basis_unit microgram        = mass_unit(               1,       1000000);
                /** basis unit of 10^-3 grams **/
                constexpr basis_unit milligram        = mass_unit(               1,          1000);
                /** basis unit of 1 gram **/
                constexpr basis_unit gram             = mass_unit(               1,             1);
                /** basis unit of 10^3 grams **/
                constexpr basis_unit kilogram         = mass_unit(            1000,             1);
                /** basis unit of 10^6 grams = 10^3 kilograms **/
                constexpr basis_unit tonne            = mass_unit(         1000000,             1);
                /** basis unit of 10^9 grams = 10^6 kilograms = 10^3 tonnes **/
                constexpr basis_unit kilotonne        = mass_unit(      1000000000,             1);
                /** basis unit of 10^12 grams = 10^9 kilograms = 10^6 tonnes **/
                constexpr basis_unit megatonne        = mass_unit(   1000000000000,             1);
                /** basis unit of 10^15 grams = 10^12 kilograms = 10^9 tonnes **/
                constexpr basis_unit gigatonne        = mass_unit(1000000000000000,             1);
                ///@}

                // ----- distance -----

                constexpr basis_unit distance_unit(std::int64_t num, std::int64_t den) {
                    return basis_unit(dimension::distance, scalefactor_ratio_type(num, den));
                }

                /** @defgroup basis-unit-distance-units basis_unit distance units **/
                ///@{
                /* US spelling */
                /** basis unit of 10^-12 meters **/
                constexpr basis_unit picometer        = distance_unit(            1, 1000000000000);
                /** basis unit of 10^-9 meters **/
                constexpr basis_unit nanometer        = distance_unit(            1,    1000000000);
                /** basis unit of 10^-6 meters **/
                constexpr basis_unit micrometer       = distance_unit(            1,       1000000);
                /** basis unit of 10^-3 meters **/
                constexpr basis_unit millimeter       = distance_unit(            1,          1000);
                /** basis unit of 1 meter **/
                constexpr basis_unit meter            = distance_unit(            1,             1);
                /** basis unit of 10^3 meters **/
                constexpr basis_unit kilometer        = distance_unit(         1000,             1);
                /** basis unit of 10^6 meters (for form's sake -- not commonly used) **/
                constexpr basis_unit megameter        = distance_unit(      1000000,             1);
                /** basis unit of 10^9 meters (for form's sake -- not commonly used) **/
                constexpr basis_unit gigameter        = distance_unit(   1000000000,             1);

                /** basis unit of 1 light-second = distance light travels in a vacuum in 1 second **/
                constexpr basis_unit lightsecond      = distance_unit(    299792458,             1);
                /** basis unit of 1 astronomical unit, representing approximate radius of earth orbit. **/
                constexpr basis_unit astronomicalunit = distance_unit( 149597870700,             1);

                /* Int'l spelling */
                /** international spelling for picometer **/
                constexpr basis_unit picometre        = picometer;
                /** international spelling for nanometer **/
                constexpr basis_unit nanometre        = nanometer;
                /** international spelling for micrometer **/
                constexpr basis_unit micrometre       = micrometer;
                /** international spelling for millimeter **/
                constexpr basis_unit millimetre       = millimeter;
                /** international spelling for meter **/
                constexpr basis_unit metre            = meter;
                /** international spelling for kilometer **/
                constexpr basis_unit kilometre        = kilometer;
                /** international spelling for megameter **/
                constexpr basis_unit megametre        = megameter;
                /** international spelling for gigameter **/
                constexpr basis_unit gigametre        = gigameter;

                /** @brief basis-unit representing 1 inch; defined as exactly 1/12 feet **/
                constexpr basis_unit inch             = distance_unit(       3048, 120000);
                /** @brief basis-unit representing 1 foot; defined as exactly 0.3048 meters **/
                constexpr basis_unit foot             = distance_unit(       3048,  10000);
                /** @brief basis-unit representing 1 yard; defined as exactly 3 feet **/
                constexpr basis_unit yard             = distance_unit(     3*3048,  10000);
                /** @brief basis-unit representing 1 mile; defined as exactly 1760 yards = 5280 feet **/
                constexpr basis_unit mile             = distance_unit(  5280*3048,  10000);
                ///@}

                // ----- time -----

                constexpr basis_unit time_unit(std::int64_t num, std::int64_t den) {
                    return basis_unit(dimension::time, scalefactor_ratio_type(num, den));
                }

                /** @defgroup basis-unit-time-units basis_unit time units **/
                ///@{
                /** basis unit of 10^-12 seconds **/
                constexpr basis_unit picosecond       = time_unit(                   1, 1000000000000);
                /** basis unit of 10^-9 seconds **/
                constexpr basis_unit nanosecond       = time_unit(                   1,    1000000000);
                /** basis unit of 10^-6 seconds **/
                constexpr basis_unit microsecond      = time_unit(                   1,       1000000);
                /** basis unit of 10^-3 seconds **/
                constexpr basis_unit millisecond      = time_unit(                   1,          1000);
                /** basis unit of 1 second **/
                constexpr basis_unit second           = time_unit(                   1,             1);
                /** basis unit of 1 minute = 60 seconds **/
                constexpr basis_unit minute           = time_unit(                  60,             1);
                /** basis unit of 1 hour = 3600 seconds **/
                constexpr basis_unit hour             = time_unit(                3600,             1);
                /** basis unit of 1 day = exactly 24 hours **/
                constexpr basis_unit day              = time_unit(             24*3600,             1);
                /** basis unit of 1 week = exactly 7 days **/
                constexpr basis_unit week             = time_unit(           7*24*3600,             1);
                /** basis unit of 1 month = exactly 30 days **/
                constexpr basis_unit month            = time_unit(          30*24*3600,             1);
                /** basis unit of 1 year, defined as 365.25 days **/
                constexpr basis_unit year             = time_unit(     (365*24+6)*3600,             1);

                /* alt conventions used in finance */
                /** basis unit of 1 year365 = exactly 365 days **/
                constexpr basis_unit year365          = time_unit(         365*24*3600,             1);
                /** basis unit of 1 year360 = exactly 360 days **/
                constexpr basis_unit year360          = time_unit(         360*24*3600,             1);
                /** basis unit of 1 year250 = exactly 250 days.
                 *  Approximate number of business days in one year
                 **/
                constexpr basis_unit year250          = time_unit(         250*24*3600,             1);

                //constexpr basis_unit century          = time_unit( 100L*(365*24+6)*3600,             1);
                //constexpr basis_unit millenium        = time_unit(1000L*(365*24+6)*3600,             1);
                ///@}

                // ----- currency -----

                /** @defgroup basis-unit-misc-units basis_unit miscellaneous units**/
                ///@{

                constexpr basis_unit currency_unit(std::int64_t num, std::int64_t den) {
                    return basis_unit(dimension::currency, scalefactor_ratio_type(num, den));
                }

                /** pseudounit -- placeholder for any actual currency amount **/
                constexpr basis_unit currency = currency_unit(1, 1);

                // ----- price -----

                constexpr basis_unit price_unit(std::int64_t num, std::int64_t den) {
                    return basis_unit(dimension::price, scalefactor_ratio_type(num, den));
                }

                /** psuedounit -- context-dependent interpretation for a screen price **/
                constexpr basis_unit price = price_unit(1, 1);
                ///@}
            } /*namespace bu*/
        } /*namespace detail*/
    } /*namespace qty*/
} /*namespace xo*/

/** end basis_unit.hpp **/
