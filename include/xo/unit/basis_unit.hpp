/** @file basis_unit.hpp **/

#pragma once

#include "dimension.hpp"
#include "basis_unit_abbrev.hpp"
#include "xo/ratio/ratio.hpp"

namespace xo {
    namespace qty {
        /** @class basis_unit2
         *  @brief A dimensionless multiple of a single natively-specified basis dimension
         *
         *  For example "3600 minutes" or "1e-6 grams"
         **/
        struct basis_unit {
        public:
            constexpr basis_unit() = default;
            constexpr basis_unit(dimension native_dim,
                                 const scalefactor_ratio_type & scalefactor)
                : native_dim_{native_dim},
                  scalefactor_{scalefactor}
                {}

            constexpr dimension native_dim() const { return native_dim_; }
            constexpr const scalefactor_ratio_type & scalefactor() const { return scalefactor_; }

            constexpr basis_unit2_abbrev_type abbrev() const {
                return abbrev::basis_unit2_abbrev(native_dim_,
                                                  scalefactor_);
            }

            constexpr basis_unit & operator=(const basis_unit & x) = default;

        public: /* need public members so that a basis_unit instance can be a non-type template parameter (a structural type) */
            /** @brief identifies a native unit, e.g. time (in seconds) **/
            dimension native_dim_ = dimension::invalid;
            /** @brief this unit defined as multiple scalefactor times native unit **/
            scalefactor_ratio_type scalefactor_;
        };

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

        namespace bu {
            // ----- mass -----

            constexpr basis_unit picogram         = basis_unit(dim::mass, scalefactor_ratio_type(               1, 1000000000000));
            constexpr basis_unit nanogram         = basis_unit(dim::mass, scalefactor_ratio_type(               1,    1000000000));
            constexpr basis_unit microgram        = basis_unit(dim::mass, scalefactor_ratio_type(               1,       1000000));
            constexpr basis_unit milligram        = basis_unit(dim::mass, scalefactor_ratio_type(               1,          1000));
            constexpr basis_unit gram             = basis_unit(dim::mass, scalefactor_ratio_type(               1,             1));
            constexpr basis_unit kilogram         = basis_unit(dim::mass, scalefactor_ratio_type(            1000,             1));
            constexpr basis_unit tonne            = basis_unit(dim::mass, scalefactor_ratio_type(         1000000,             1));
            constexpr basis_unit kilotonne        = basis_unit(dim::mass, scalefactor_ratio_type(      1000000000,             1));
            constexpr basis_unit megatonne        = basis_unit(dim::mass, scalefactor_ratio_type(   1000000000000,             1));
            constexpr basis_unit gigatonne        = basis_unit(dim::mass, scalefactor_ratio_type(1000000000000000,             1));

            // ----- distance -----

            /* International spelling */
            constexpr basis_unit picometre        = basis_unit(dim::distance, scalefactor_ratio_type(            1, 1000000000000));
            constexpr basis_unit nanometre        = basis_unit(dim::distance, scalefactor_ratio_type(            1,    1000000000));
            constexpr basis_unit micrometre       = basis_unit(dim::distance, scalefactor_ratio_type(            1,       1000000));
            constexpr basis_unit millimetre       = basis_unit(dim::distance, scalefactor_ratio_type(            1,          1000));
            constexpr basis_unit metre            = basis_unit(dim::distance, scalefactor_ratio_type(            1,             1));
            constexpr basis_unit kilometre        = basis_unit(dim::distance, scalefactor_ratio_type(         1000,             1));
            constexpr basis_unit megametre        = basis_unit(dim::distance, scalefactor_ratio_type(      1000000,             1));
            constexpr basis_unit gigametre        = basis_unit(dim::distance, scalefactor_ratio_type(   1000000000,             1));

            constexpr basis_unit lightsecond      = basis_unit(dim::distance, scalefactor_ratio_type(    299792458,             1));
            constexpr basis_unit astronomicalunit = basis_unit(dim::distance, scalefactor_ratio_type( 149597870700,             1));

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
            constexpr basis_unit inch             = basis_unit(dim::distance, scalefactor_ratio_type(       3048, 120000));
            /** @brief basis-unit representing 1 foot; defined as exactly 0.3048 meters **/
            constexpr basis_unit foot             = basis_unit(dim::distance, scalefactor_ratio_type(       3048,  10000));
            /** @brief basis-unit representing 1 yard; defined as exactly 3 feet **/
            constexpr basis_unit yard             = basis_unit(dim::distance, scalefactor_ratio_type(     3*3048,  10000));
            /** @brief basis-unit representing 1 mile; defined as exactly 1760 yards = 5280 feet **/
            constexpr basis_unit mile             = basis_unit(dim::distance, scalefactor_ratio_type(  5280*3048,  10000));

            // ----- time -----

            constexpr basis_unit picosecond       = basis_unit(dim::time,     scalefactor_ratio_type(            1, 1000000000000));
            constexpr basis_unit nanosecond       = basis_unit(dim::time,     scalefactor_ratio_type(            1,    1000000000));
            constexpr basis_unit microsecond      = basis_unit(dim::time,     scalefactor_ratio_type(            1,       1000000));
            constexpr basis_unit millisecond      = basis_unit(dim::time,     scalefactor_ratio_type(            1,          1000));
            constexpr basis_unit second           = basis_unit(dim::time,     scalefactor_ratio_type(            1,             1));
            constexpr basis_unit minute           = basis_unit(dim::time,     scalefactor_ratio_type(           60,             1));
            constexpr basis_unit hour             = basis_unit(dim::time,     scalefactor_ratio_type(         3600,             1));
            constexpr basis_unit day              = basis_unit(dim::time,     scalefactor_ratio_type(      24*3600,             1));
            constexpr basis_unit week             = basis_unit(dim::time,     scalefactor_ratio_type(    7*24*3600,             1));
            constexpr basis_unit month            = basis_unit(dim::time,     scalefactor_ratio_type(   30*24*3600,             1));
            constexpr basis_unit year             = basis_unit(dim::time,     scalefactor_ratio_type((365*24+6)*3600,           1));

            /* alt conventions used in finance */
            constexpr basis_unit year365          = basis_unit(dim::time,     scalefactor_ratio_type(  365*24*3600,             1));
            constexpr basis_unit year360          = basis_unit(dim::time,     scalefactor_ratio_type(  360*24*3600,             1));
            /* 250 = approx number of trading days in a calendar year */
            constexpr basis_unit year250          = basis_unit(dim::time,     scalefactor_ratio_type(  250*24*3600,             1));

            // ----- currency -----

            /* pseudounit -- placeholder for any actual currency amount */
            constexpr basis_unit currency         = basis_unit(dim::currency, scalefactor_ratio_type(            1,             1));

            // ----- price -----

            /* psuedounit -- context-dependent interpretation */
            constexpr basis_unit price            = basis_unit(dim::price,    scalefactor_ratio_type(            1,             1));
        }

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

            // ----- scaled_native_unit_abbrev_helper -----

            /* Require: InnerScale is ratio type; InnerScale >= 1
             *
             * NOTE: clang 18 doesn't accept that scalefactor_ratio_type is a 'structural type'
             */
            template <dim BasisDim, std::int64_t InnerScaleNum = 1, std::int64_t InnerScaleDen = 1>
            struct scaled_native_unit2_abbrev;

            template <dim BasisDim>
            struct scaled_native_unit2_abbrev<BasisDim, 1, 1> {
                static constexpr const basis_unit2_abbrev_type value
                = (basis_unit2_abbrev_type::from_flatstring
                   (native_unit2_v[static_cast<uint32_t>(BasisDim)]
                    .abbrev_str()));
            };

            inline
            constexpr basis_unit2_abbrev_type
            bu_fallback_abbrev(dim basis_dim,
                               const scalefactor_ratio_type & scalefactor)
            {
                return (basis_unit2_abbrev_type::from_flatstring
                        (flatstring_concat
                         (scalefactor.to_str<basis_unit2_abbrev_type::fixed_capacity>(),
                          native_unit2_v[static_cast<std::uint32_t>(basis_dim)].abbrev_str())));
            }
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end basis_unit.hpp **/
