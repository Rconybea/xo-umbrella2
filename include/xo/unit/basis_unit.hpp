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
         *  For example "3600 minutes" or "1e-6 grams"
         **/
        struct basis_unit {
        public:
            /** @defgroup basis-unit-constructors basis_unit constructors **/
            ///@{
            constexpr basis_unit() = default;
            constexpr basis_unit(dimension native_dim,
                                 const scalefactor_ratio_type & scalefactor)
                : native_dim_{native_dim},
                  scalefactor_{scalefactor}
                {}
            ///@}

            /** @defgroup basis-unit-access-methods basis_unit access methods **/
            ///@{
            constexpr dimension native_dim() const { return native_dim_; }
            constexpr const scalefactor_ratio_type & scalefactor() const { return scalefactor_; }
            ///@}

#ifdef OBSOLETE // use bu_abbrev(bu)
           constexpr bu_abbrev_type abbrev() const {
                return basis_unit2_abbrev(native_dim_, scalefactor_);
            }
#endif

        public: /* public so instance can be a non-type template parameter (a structural type) */
            /** @defgroup basis-unit-instance-vars **/
            ///@{
            /** @brief identifies a native unit, e.g. time (in seconds) **/
            dimension native_dim_ = dimension::invalid;
            /** @brief this unit defined as multiple scalefactor times native unit **/
            scalefactor_ratio_type scalefactor_;
            ///@}
        };

        /** @defgroup basis-unit-comparison-support basis_unit comparisons **/
        ///@{
        inline constexpr bool
        operator==(const basis_unit & x, const basis_unit & y)
        {
            return ((x.native_dim_ == y.native_dim_)
                    && (x.scalefactor_ == y.scalefactor_));
        }

        inline constexpr bool
        operator!=(const basis_unit & x, const basis_unit & y)
        {
            return ((x.native_dim_ != y.native_dim_)
                    || (x.scalefactor_ != y.scalefactor_));
        }
        ///@}

        namespace detail {
            namespace bu {
                // ----- mass -----

                constexpr basis_unit mass_unit(std::int64_t num, std::int64_t den) {
                    return basis_unit(dimension::mass, scalefactor_ratio_type(num, den));
                }

                /** @defgroup basis-unit-mass-units **/
                ///@{
                constexpr basis_unit picogram         = mass_unit(               1, 1000000000000);
                constexpr basis_unit nanogram         = mass_unit(               1,    1000000000);
                constexpr basis_unit microgram        = mass_unit(               1,       1000000);
                constexpr basis_unit milligram        = mass_unit(               1,          1000);
                constexpr basis_unit gram             = mass_unit(               1,             1);
                constexpr basis_unit kilogram         = mass_unit(            1000,             1);
                constexpr basis_unit tonne            = mass_unit(         1000000,             1);
                constexpr basis_unit kilotonne        = mass_unit(      1000000000,             1);
                constexpr basis_unit megatonne        = mass_unit(   1000000000000,             1);
                constexpr basis_unit gigatonne        = mass_unit(1000000000000000,             1);
                ///@}

                // ----- distance -----

                constexpr basis_unit distance_unit(std::int64_t num, std::int64_t den) {
                    return basis_unit(dimension::distance, scalefactor_ratio_type(num, den));
                }

                /** @defgroup basis-unit-distance-units **/
                ///@{
                /* International spelling */
                constexpr basis_unit picometre        = distance_unit(            1, 1000000000000);
                constexpr basis_unit nanometre        = distance_unit(            1,    1000000000);
                constexpr basis_unit micrometre       = distance_unit(            1,       1000000);
                constexpr basis_unit millimetre       = distance_unit(            1,          1000);
                constexpr basis_unit metre            = distance_unit(            1,             1);
                constexpr basis_unit kilometre        = distance_unit(         1000,             1);
                constexpr basis_unit megametre        = distance_unit(      1000000,             1);
                constexpr basis_unit gigametre        = distance_unit(   1000000000,             1);

                constexpr basis_unit lightsecond      = distance_unit(    299792458,             1);
                constexpr basis_unit astronomicalunit = distance_unit( 149597870700,             1);

                /* US spelling */
                constexpr basis_unit picometer        = picometre;
                constexpr basis_unit nanometer        = nanometre;
                constexpr basis_unit micrometer       = micrometre;
                constexpr basis_unit millimeter       = millimetre;
                constexpr basis_unit meter            = metre;
                constexpr basis_unit kilometer        = kilometre;
                constexpr basis_unit megameter        = megametre;
                constexpr basis_unit gigameter        = gigametre;

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

                /** @defgroup basis-unit-time-units **/
                ///@{
                constexpr basis_unit picosecond       = time_unit(                   1, 1000000000000);
                constexpr basis_unit nanosecond       = time_unit(                   1,    1000000000);
                constexpr basis_unit microsecond      = time_unit(                   1,       1000000);
                constexpr basis_unit millisecond      = time_unit(                   1,          1000);
                constexpr basis_unit second           = time_unit(                   1,             1);
                constexpr basis_unit minute           = time_unit(                  60,             1);
                constexpr basis_unit hour             = time_unit(                3600,             1);
                constexpr basis_unit day              = time_unit(             24*3600,             1);
                constexpr basis_unit week             = time_unit(           7*24*3600,             1);
                constexpr basis_unit month            = time_unit(          30*24*3600,             1);
                constexpr basis_unit year             = time_unit(     (365*24+6)*3600,             1);

                /* alt conventions used in finance */
                constexpr basis_unit year365          = time_unit(         365*24*3600,             1);
                constexpr basis_unit year360          = time_unit(         360*24*3600,             1);
                /* 250 = approx number of trading days in a calendar year */
                constexpr basis_unit year250          = time_unit(         250*24*3600,             1);

                constexpr basis_unit century          = time_unit( 100L*(365*24+6)*3600,             1);
                constexpr basis_unit millenium        = time_unit(1000L*(365*24+6)*3600,             1);
                ///@}

                // ----- currency -----

                /** @defgroup basis-unit-misc-units **/
                ///@{

                /* pseudounit -- placeholder for any actual currency amount */
                constexpr basis_unit currency         = basis_unit(dim::currency, scalefactor_ratio_type(            1,             1));

                // ----- price -----

                /* psuedounit -- context-dependent interpretation */
                constexpr basis_unit price            = basis_unit(dim::price,    scalefactor_ratio_type(            1,             1));
                ///@}
            } /*namespace bu*/
        } /*namespace detail*/

        namespace units {
            /** for runtime work,  would like to be able to promptly find special abbreviation
             *  keyed by (native_dim, scalefactor).
             *
             *  Also want to support compile-time-only unit computation.
             *  Since constexpr unordered containers aren't obviously feasible (as of c++23).
             *
             *  Solution adopted here is to support introduction of scaled native units
             *  only at compile time,  at least for now
             **/

            // ----- bu_fallback_abbrev -----

            /* Require: InnerScale is ratio type; InnerScale >= 1 */
            template <dim BasisDim, std::int64_t InnerScaleNum = 1, std::int64_t InnerScaleDen = 1>
            struct scaled_native_unit2_abbrev;

            template <dim BasisDim>
            struct scaled_native_unit2_abbrev<BasisDim, 1, 1> {
                static constexpr const bu_abbrev_type value
                = (bu_abbrev_type::from_flatstring
                   (native_unit2_v[static_cast<uint32_t>(BasisDim)]
                    .abbrev_str()));
            };

            inline
            constexpr bu_abbrev_type
            bu_fallback_abbrev(dim basis_dim,
                               const scalefactor_ratio_type & scalefactor)
            {
                return (bu_abbrev_type::from_flatstring
                        (flatstring_concat
                         (scalefactor.to_str<bu_abbrev_type::fixed_capacity>(),
                          native_unit2_v[static_cast<std::uint32_t>(basis_dim)].abbrev_str())));
            }
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end basis_unit.hpp **/
