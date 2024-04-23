/** @file basis_unit2.hpp **/

#pragma once

#include "native_unit.hpp"
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
            constexpr basis_unit(dim native_dim, const scalefactor_ratio_type & scalefactor)
                : native_dim_{native_dim},
                  scalefactor_{scalefactor}
                {}

            constexpr dim native_dim() const { return native_dim_; }
            constexpr const scalefactor_ratio_type & scalefactor() const { return scalefactor_; }

            constexpr basis_unit2_abbrev_type abbrev() const {
                return abbrev::basis_unit2_abbrev(native_dim_,
                                                  scalefactor_);
            }

            constexpr basis_unit & operator=(const basis_unit & x) = default;

            /** @brief identifies a native unit, e.g. time (in seconds) **/
            dim native_dim_ = dim::invalid;
            /** @brief this unit defined as multiple scalefactor times native unit **/
            scalefactor_ratio_type scalefactor_;
        };

        namespace bu2 {
            constexpr basis_unit nanogram  = basis_unit(dim::mass, scalefactor_ratio_type(1,    1000000000));
            constexpr basis_unit microgram = basis_unit(dim::mass, scalefactor_ratio_type(1,    1000000));
            constexpr basis_unit milligram = basis_unit(dim::mass, scalefactor_ratio_type(1,    1000));
            constexpr basis_unit gram      = basis_unit(dim::mass, scalefactor_ratio_type(1,    1));
            constexpr basis_unit kilogram  = basis_unit(dim::mass, scalefactor_ratio_type(1000, 1));
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

            template <dim BasisDim,
                      std::int64_t InnerScaleNum,
                      std::int64_t InnerScaleDen>
            struct scaled_native_unit2_abbrev {
                /* e.g. unit of '1000 grams' will have abbrev '1000g' in absence
                 *      of a specialization for scaled_native_unit_abbrev
                 */
                static constexpr const basis_unit2_abbrev_type value
                = (basis_unit2_abbrev_type::from_flatstring
                   (flatstring_concat
                    (xo::ratio::ratio<std::int64_t>(InnerScaleNum,
                                                    InnerScaleDen)
                     .to_str<basis_unit2_abbrev_type::fixed_capacity>(),
                     native_unit2_v[static_cast<std::uint32_t>(BasisDim)].abbrev_str())));

                // = bu_fallback_abbrev(BasisDim,
                //                      xo::ratio::ratio<std::int64_t>(InnerScaleNum, InnerScaleDen));
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::mass, bu2::nanogram.scalefactor().num(), bu2::nanogram.scalefactor().den()> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("ng");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::mass, 1, 1000000> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("ug");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::mass, 1, 1000> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("mg");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::mass, 1000, 1> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("kg");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::mass, 1000000, 1> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("t");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::mass, 1000000000, 1> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("kt");
            };

            // ----- units for dim::distance -----

            template <>
            struct scaled_native_unit2_abbrev<dim::distance, 1, 1000000000> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("nm");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::distance, 1, 1000000> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("um");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::distance, 1, 1000> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("mm");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::distance, 1000, 1> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("km");
            };

            // ----- units for dim::time -----

            template <>
            struct scaled_native_unit2_abbrev<dim::time, 1, 1000000000> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("ns");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::time, 1, 1000000> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("us");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::time, 1, 1000> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("ms");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::time, 60, 1> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("min");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::time, 3600, 1> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("hr");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::time, 24*3600, 1> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("dy");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::time, 250*24*3600, 1> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("yr250");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::time, 360*24*3600, 1> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("yr360");
            };

            template <>
            struct scaled_native_unit2_abbrev<dim::time, 365*24*3600, 1> {
                static constexpr const basis_unit2_abbrev_type value = basis_unit2_abbrev_type::from_chars("yr365");
            };

            // ----- native unit abbrev api -----

            template <dim BasisDim, std::int64_t InnerScaleNum = 1, std::int64_t InnerScaleDen = 1>
            constexpr auto scaled_native_unit2_abbrev_v = scaled_native_unit2_abbrev<BasisDim, InnerScaleNum, InnerScaleDen>::value;
        }

    } /*namespace qty*/
} /*namespace xo*/


/** end basis_unit2.hpp **/
