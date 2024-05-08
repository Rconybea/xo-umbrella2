/** @file basis_unit_abbrev.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "native_unit.hpp"
#include "xo/ratio/ratio.hpp"
//#include <cstdint>

namespace xo {
    namespace qty {
        using bu_abbrev_type = flatstring<16>;
        using scalefactor_ratio_type = xo::ratio::ratio<std::int64_t>;

        namespace abbrev {
            static
            constexpr bu_abbrev_type
            fallback_unit_abbrev(const scalefactor_ratio_type & scalefactor,
                                 dimension native_dim)
            {
                /* e.g. unit of '1000 grams' will have abbrev '1000g' in absence
                 *      of a specialization for scaled_native_unit_abbrev
                 */
                return (bu_abbrev_type::from_flatstring
                        (flatstring_concat
                         (scalefactor.to_str<bu_abbrev_type::fixed_capacity>(),
                          native_unit2_v[static_cast<std::uint32_t>(native_dim)].abbrev_str())));
            }

            // ----- units for dim::mass -----

            /** @brief return abbreviated suffix to use for a mass unit
             *  of (relative) size @p scalefactor.
             *
             *  Example:
             *  @code
             *  using namespace xo::qty;
             *
             *  static_assert(abbrev::mass_unit2_abbrev(scalefactor_ratio_type(1,1000))
             *                == xo::flatstring("mg"));
             *  @endcode
             **/
            static
            constexpr bu_abbrev_type
            mass_unit2_abbrev(const scalefactor_ratio_type & scalefactor)
            {
                if (scalefactor.num() == 1) {
                    switch (scalefactor.den()) {
                    case 1:
                        return bu_abbrev_type::from_chars("g");
                    case 1000:
                        return bu_abbrev_type::from_chars("mg");
                    case 1000000:
                        return bu_abbrev_type::from_chars("ug");
                    case 1000000000:
                        return bu_abbrev_type::from_chars("ng");
                    case 1000000000000:
                        return bu_abbrev_type::from_chars("pg");
                    }
                }

                if (scalefactor.den() == 1) {
                    switch (scalefactor.num()) {
                    case 1000:
                        return bu_abbrev_type::from_chars("kg");
                    case 1000000:
                        return bu_abbrev_type::from_chars("t");
                    case 1000000000:
                        return bu_abbrev_type::from_chars("kt");
                    case 1000000000000:
                        return bu_abbrev_type::from_chars("Mt");
                    case 1000000000000000:
                        return bu_abbrev_type::from_chars("Gt");
                    }
                }

                return fallback_unit_abbrev(scalefactor, dimension::mass);
            }

            // ----- units for dim::distance -----

            static
            constexpr bu_abbrev_type
            distance_unit2_abbrev(const scalefactor_ratio_type & scalefactor)
            {
                if (scalefactor.num() == 1) {
                    switch (scalefactor.den()) {
                    case 1:
                        return bu_abbrev_type::from_chars("m");
                    case 1000:
                        return bu_abbrev_type::from_chars("mm");
                    case 1000000:
                        return bu_abbrev_type::from_chars("um");
                    case 1000000000:
                        return bu_abbrev_type::from_chars("nm");
                    case 1000000000000:
                        return bu_abbrev_type::from_chars("pm");
                    }
                }

                if (scalefactor.den() == 1) {
                    switch (scalefactor.num()) {
                    case 1000:
                        return bu_abbrev_type::from_chars("km");
                    case 1000000:
                        return bu_abbrev_type::from_chars("Mm");
                    case 299792458:
                        return bu_abbrev_type::from_chars("lsec");
                    case 1000000000:
                        return bu_abbrev_type::from_chars("Gm");
                    case 149597870700:
                        return bu_abbrev_type::from_chars("AU");
                    }
                }

                if (scalefactor.den() == 10000) {
                    switch(scalefactor.num()) {
                    case 3048:
                        return bu_abbrev_type::from_chars("ft");
                    case 3*3048:
                        return bu_abbrev_type::from_chars("yd");
                    case 5280*3048:
                        return bu_abbrev_type::from_chars("mi");
                    }
                }

                if (scalefactor.num() == 3048 && scalefactor.den() == 120000)
                    return bu_abbrev_type::from_chars("in");

                return fallback_unit_abbrev(scalefactor, dimension::distance);
            }

            // ----- units for dim::time -----

            static
            constexpr bu_abbrev_type
            time_unit2_abbrev(const scalefactor_ratio_type & scalefactor)
            {
                if (scalefactor.num() == 1) {
                    switch (scalefactor.den()) {
                    case 1:
                        return bu_abbrev_type::from_chars("s");
                    case 1000:
                        return bu_abbrev_type::from_chars("ms");
                    case 1000000:
                        return bu_abbrev_type::from_chars("us");
                    case 1000000000:
                        return bu_abbrev_type::from_chars("ns");
                    case 1000000000000:
                        return bu_abbrev_type::from_chars("ps");
                    }
                }

                if (scalefactor.den() == 1) {
                    switch (scalefactor.num()) {
                    case 60:
                        return bu_abbrev_type::from_chars("min");
                    case 3600:
                        return bu_abbrev_type::from_chars("hr");
                    case 24*3600:
                        return bu_abbrev_type::from_chars("dy");
                    case 7*24*3600:
                        return bu_abbrev_type::from_chars("wk");
                    case 30*24*3600:
                        return bu_abbrev_type::from_chars("mo");
                    case 250*24*3600:
                        return bu_abbrev_type::from_chars("yr250");
                    case 360*24*3600:
                        return bu_abbrev_type::from_chars("yr360");
                    case 365*24*3600:
                        return bu_abbrev_type::from_chars("yr365");
                    case 365*24*3600+6*3600:
                        return bu_abbrev_type::from_chars("yr");
                    }
                }

                return fallback_unit_abbrev(scalefactor, dimension::time);
            }

            // ----- units for dim::currency -----

            static constexpr bu_abbrev_type
            currency_unit2_abbrev(const scalefactor_ratio_type & scalefactor)
            {
                if (scalefactor.num() == 1) {
                    switch(scalefactor.den()) {
                    case 1:
                        return bu_abbrev_type::from_chars("ccy");
                    }
                }

                return fallback_unit_abbrev(scalefactor, dimension::currency);
            }

            // ----- units for dim::price -----

            static constexpr bu_abbrev_type
            price_unit2_abbrev(const scalefactor_ratio_type & scalefactor)
            {
                if (scalefactor.num() == 1) {
                    switch(scalefactor.den()) {
                    case 1:
                        return bu_abbrev_type::from_chars("px");
                    }
                }

                return fallback_unit_abbrev(scalefactor, dimension::price);
            }

            // ----- basis_unit2_abbrev -----

            static
            constexpr bu_abbrev_type
            basis_unit2_abbrev(dimension native_dim,
                               const scalefactor_ratio_type & scalefactor)
            {
                switch(native_dim) {
                case dimension::mass:
                    return mass_unit2_abbrev(scalefactor);
                case dimension::distance:
                    return distance_unit2_abbrev(scalefactor);
                case dimension::time:
                    return time_unit2_abbrev(scalefactor);
                case dimension::currency:
                    return currency_unit2_abbrev(scalefactor);
                case dimension::price:
                    return price_unit2_abbrev(scalefactor);
                case dimension::invalid:
                case dimension::n_dim:
                    return bu_abbrev_type();
                    break;
                }

                /* unreachable (for well-constructed dim instances) */

                return bu_abbrev_type();
            }
        } /*namespace abbrev*/
    } /*namespace qty*/
} /*namespace xo*/


/** end basis_unit_abbrev.hpp **/
