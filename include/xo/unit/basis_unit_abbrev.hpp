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
        using basis_unit2_abbrev_type = flatstring<16>;
        using scalefactor_ratio_type = xo::ratio::ratio<std::int64_t>;

        namespace abbrev {
            static
            constexpr basis_unit2_abbrev_type
            fallback_unit_abbrev(const scalefactor_ratio_type & scalefactor,
                                 dim native_dim)
            {
                /* e.g. unit of '1000 grams' will have abbrev '1000g' in absence
                 *      of a specialization for scaled_native_unit_abbrev
                 */
                return (basis_unit2_abbrev_type::from_flatstring
                        (flatstring_concat
                         (scalefactor.to_str<basis_unit2_abbrev_type::fixed_capacity>(),
                          native_unit2_v[static_cast<std::uint32_t>(native_dim)].abbrev_str())));
            }

            // ----- units for dim::mass -----

            static
            constexpr basis_unit2_abbrev_type
            mass_unit2_abbrev(const scalefactor_ratio_type & scalefactor)
            {
                if (scalefactor.num() == 1) {
                    switch (scalefactor.den()) {
                    case 1:
                        return basis_unit2_abbrev_type::from_chars("g");
                    case 1000:
                        return basis_unit2_abbrev_type::from_chars("mg");
                    case 1000000:
                        return basis_unit2_abbrev_type::from_chars("ug");
                    case 1000000000:
                        return basis_unit2_abbrev_type::from_chars("ng");
                    case 1000000000000:
                        return basis_unit2_abbrev_type::from_chars("pg");
                    }
                }

                if (scalefactor.den() == 1) {
                    switch (scalefactor.num()) {
                    case 1000:
                        return basis_unit2_abbrev_type::from_chars("kg");
                    case 1000000:
                        return basis_unit2_abbrev_type::from_chars("t");
                    case 1000000000:
                        return basis_unit2_abbrev_type::from_chars("kt");
                    case 1000000000000:
                        return basis_unit2_abbrev_type::from_chars("Mt");
                    }
                }

                return fallback_unit_abbrev(scalefactor, dim::mass);
            }

            // ----- units for dim::distance -----

            static
            constexpr basis_unit2_abbrev_type
            distance_unit2_abbrev(const scalefactor_ratio_type & scalefactor)
            {
                if (scalefactor.num() == 1) {
                    switch (scalefactor.den()) {
                    case 1:
                        return basis_unit2_abbrev_type::from_chars("m");
                    case 1000:
                        return basis_unit2_abbrev_type::from_chars("mm");
                    case 1000000:
                        return basis_unit2_abbrev_type::from_chars("um");
                    case 1000000000:
                        return basis_unit2_abbrev_type::from_chars("nm");
                    case 1000000000000:
                        return basis_unit2_abbrev_type::from_chars("pm");
                    }
                }

                if (scalefactor.den() == 1) {
                    switch (scalefactor.num()) {
                    case 1000:
                        return basis_unit2_abbrev_type::from_chars("km");
                    case 1000000:
                        return basis_unit2_abbrev_type::from_chars("Mm");
                    case 299792458:
                        return basis_unit2_abbrev_type::from_chars("lsec");
                    case 1000000000:
                        return basis_unit2_abbrev_type::from_chars("Gm");
                    case 149597870700:
                        return basis_unit2_abbrev_type::from_chars("AU");
                    }
                }

                return fallback_unit_abbrev(scalefactor, dim::distance);
            }

            // ----- units for dim::time -----

            static
            constexpr basis_unit2_abbrev_type
            time_unit2_abbrev(const scalefactor_ratio_type & scalefactor)
            {
                if (scalefactor.num() == 1) {
                    switch (scalefactor.den()) {
                    case 1:
                        return basis_unit2_abbrev_type::from_chars("s");
                    case 1000:
                        return basis_unit2_abbrev_type::from_chars("ms");
                    case 1000000:
                        return basis_unit2_abbrev_type::from_chars("us");
                    case 1000000000:
                        return basis_unit2_abbrev_type::from_chars("ns");
                    case 1000000000000:
                        return basis_unit2_abbrev_type::from_chars("ps");
                    }
                }

                if (scalefactor.den() == 1) {
                    switch (scalefactor.num()) {
                    case 60:
                        return basis_unit2_abbrev_type::from_chars("min");
                    case 3600:
                        return basis_unit2_abbrev_type::from_chars("hr");
                    case 24*3600:
                        return basis_unit2_abbrev_type::from_chars("dy");
                    case 7*24*3600:
                        return basis_unit2_abbrev_type::from_chars("wk");
                    case 30*24*3600:
                        return basis_unit2_abbrev_type::from_chars("mo");
                    case 250*24*3600:
                        return basis_unit2_abbrev_type::from_chars("yr250");
                    case 360*24*3600:
                        return basis_unit2_abbrev_type::from_chars("yr360");
                    case 365*24*3600:
                        return basis_unit2_abbrev_type::from_chars("yr365");
                    case 365*24*3600+6*3600:
                        return basis_unit2_abbrev_type::from_chars("yr");
                    }
                }

                return fallback_unit_abbrev(scalefactor, dim::time);
            }

            // ----- units for dim::currency -----

            static constexpr basis_unit2_abbrev_type
            currency_unit2_abbrev(const scalefactor_ratio_type & scalefactor)
            {
                if (scalefactor.num() == 1) {
                    switch(scalefactor.den()) {
                    case 1:
                        return basis_unit2_abbrev_type::from_chars("ccy");
                    }
                }

                return fallback_unit_abbrev(scalefactor, dim::currency);
            }

            // ----- units for dim::price -----

            static constexpr basis_unit2_abbrev_type
            price_unit2_abbrev(const scalefactor_ratio_type & scalefactor)
            {
                if (scalefactor.num() == 1) {
                    switch(scalefactor.den()) {
                    case 1:
                        return basis_unit2_abbrev_type::from_chars("px");
                    }
                }

                return fallback_unit_abbrev(scalefactor, dim::price);
            }

            // ----- basis_unit2_abbrev -----

            static
            constexpr basis_unit2_abbrev_type
            basis_unit2_abbrev(dim native_dim,
                               const scalefactor_ratio_type & scalefactor)
            {
                switch(native_dim) {
                case dim::mass:
                    return mass_unit2_abbrev(scalefactor);
                case dim::distance:
                    return distance_unit2_abbrev(scalefactor);
                case dim::time:
                    return time_unit2_abbrev(scalefactor);
                case dim::currency:
                    return currency_unit2_abbrev(scalefactor);
                case dim::price:
                    return price_unit2_abbrev(scalefactor);
                case dim::invalid:
                case dim::n_dim:
                    return basis_unit2_abbrev_type();
                    break;
                }

                /* unreachable (for well-constructed dim instances) */

                return basis_unit2_abbrev_type();
            }
        } /*namespace abbrev*/
    } /*namespace qty*/
} /*namespace xo*/


/** end basis_unit_abbrev.hpp **/
