/** @file basis_unit2.hpp **/

#pragma once

#include "dim_util2.hpp"
#include "xo/ratio/ratio.hpp"
#include <algorithm>
#include <vector>

namespace xo {
    namespace unit {
        using basis_unit2_abbrev_type = flatstring<16>;

        using scalefactor_ratio_type = xo::ratio::ratio<std::int64_t>;

        /** @class basis_unit2
         *  @brief A dimensionless multiple of a single natively-specified basis dimension
         *
         *  For example "3600 minutes" or "1e-6 grams"
         **/
        struct basis_unit2 {
        public:
            constexpr basis_unit2(dim native_dim, const scalefactor_ratio_type & scalefactor)
                : native_dim_{native_dim},
                  scalefactor_{scalefactor}
                {}

            constexpr dim native_dim() const { return native_dim_; }
            constexpr const scalefactor_ratio_type & scalefactor() const { return scalefactor_; }

            /** @brief identifies a native unit, e.g. time (in seconds) **/
            const dim native_dim_;
            /** @brief this unit defined as multiple scalefactor times native unit **/
            const scalefactor_ratio_type scalefactor_;
        };

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

            // ----- units for dim::mass -----

            template <>
            struct scaled_native_unit2_abbrev<dim::mass, 1, 1000000000> {
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

        /** @class basis_unit2_store
         *  @brief Store known basis units for runtime
         **/
        template <typename Tag>
        struct basis_unit2_store {
            basis_unit2_store() : bu_abbrev_vv_(static_cast<std::size_t>(dim::n_dim)) {
                this->bu_establish_abbrev_for<dim::mass,              1, 1000000000>();
                this->bu_establish_abbrev_for<dim::mass,              1,    1000000>();
                this->bu_establish_abbrev_for<dim::mass,              1,       1000>();
                this->bu_establish_abbrev_for<dim::mass,              1,          1>();
                this->bu_establish_abbrev_for<dim::mass,           1000,          1>();
                this->bu_establish_abbrev_for<dim::mass,        1000000,          1>();
                this->bu_establish_abbrev_for<dim::mass,     1000000000,          1>();

                this->bu_establish_abbrev_for<dim::distance,          1, 1000000000>();
                this->bu_establish_abbrev_for<dim::distance,          1,    1000000>();
                this->bu_establish_abbrev_for<dim::distance,          1,       1000>();
                this->bu_establish_abbrev_for<dim::distance,          1,          1>();
                this->bu_establish_abbrev_for<dim::distance,       1000,          1>();

                this->bu_establish_abbrev_for<dim::time,              1, 1000000000>();
                this->bu_establish_abbrev_for<dim::time,              1,    1000000>();
                this->bu_establish_abbrev_for<dim::time,              1,       1000>();
                this->bu_establish_abbrev_for<dim::time,              1,          1>();
                this->bu_establish_abbrev_for<dim::time,             60,          1>();
                this->bu_establish_abbrev_for<dim::time,           3600,          1>();
                this->bu_establish_abbrev_for<dim::time,        24*3600,          1>();
                this->bu_establish_abbrev_for<dim::time,    250*24*3600,          1>();
                this->bu_establish_abbrev_for<dim::time,    360*24*3600,          1>();
                this->bu_establish_abbrev_for<dim::time,    365*24*3600,          1>();

                this->bu_establish_abbrev_for<dim::currency,          1,          1>();

                this->bu_establish_abbrev_for<dim::price,             1,          1>();
            }

            /* e.g.
             *   [(1/1000000000, "nm"), (1/1000000, "um"), (1/1000, "mm"), (1/1, "m"), (1000/1, "km")]
             */
            using native_scale_v = std::vector<std::pair<scalefactor_ratio_type, basis_unit2_abbrev_type>>;

            /** @brief get basis-unit abbreviation at runtime **/
            basis_unit2_abbrev_type bu_abbrev(dim basis_dim,
                                              const scalefactor_ratio_type & scalefactor) const {
                const auto & bu_abbrev_v = bu_abbrev_vv_[static_cast<std::size_t>(basis_dim)];

                std::size_t i_abbrev = bu_abbrev_lub_ix(basis_dim, scalefactor, bu_abbrev_v);

                if ((i_abbrev < bu_abbrev_v.size())
                    && (bu_abbrev_v[i_abbrev].first == scalefactor))
                {
                    return bu_abbrev_v[i_abbrev].second;
                } else {
                    return units::bu_fallback_abbrev(basis_dim, scalefactor);
                }
            }

            template <dim BasisDim, std::int64_t InnerScaleNum, std::int64_t InnerScaleDen>
            void bu_establish_abbrev_for() {
                this->bu_establish_abbrev(basis_unit2(BasisDim,
                                                      scalefactor_ratio_type(InnerScaleNum, InnerScaleDen)),
                                          units::scaled_native_unit2_abbrev_v<BasisDim, InnerScaleNum, InnerScaleDen>);
            }

            /** @brief establish abbreviation @p abbrev for basis unit @p bu
             **/
            void bu_establish_abbrev(const basis_unit2 & bu,
                                     const basis_unit2_abbrev_type & abbrev) {

                auto & bu_abbrev_v = bu_abbrev_vv_[static_cast<std::size_t>(bu.native_dim())];

                std::int32_t i_abbrev = 0;

                if (!bu_abbrev_v.empty()) {
                    i_abbrev = bu_abbrev_lub_ix(bu.native_dim(),
                                                bu.scalefactor(),
                                                bu_abbrev_v);
                }

                auto entry = std::make_pair(bu.scalefactor(), abbrev);

                if ((i_abbrev < bu_abbrev_v.size())
                    && (bu_abbrev_v[i_abbrev].first == bu.scalefactor()))
                {
                    bu_abbrev_v[i_abbrev] = entry;
                } else {
                    bu_abbrev_v.insert(bu_abbrev_v.begin() + i_abbrev, entry);
                }
            }

        private:
            /** @brief get least-upper-bound index position in bu_abbrev_v[]
             *
             *  return value in [0, n] where n = bu_abbrev_v.size()
             **/
            static std::size_t bu_abbrev_lub_ix(dim basis_dim,
                                                const scalefactor_ratio_type & scalefactor,
                                                const native_scale_v & bu_abbrev_v)
                {
                    std::size_t n = bu_abbrev_v.size();

                    if (n == 0)
                        return 0;

                    std::size_t lo = 0;
                    std::size_t hi = n-1;

                    if (scalefactor <= bu_abbrev_v[lo].first)
                        return 0;

                    auto cmp = (scalefactor <=> bu_abbrev_v[hi].first);

                    if (cmp > 0)
                        return n;

                    if (cmp == 0)
                        return hi;

                    while (hi-lo > 1) {
                        /* inv:
                         *   bu_abbrev_v[lo].first < scalefactor <= bu_abbrev_v[hi].first
                         */

                        std::size_t mid = lo + (hi - lo)/2;

                        if (scalefactor > bu_abbrev_v[mid].first)
                            lo = mid;
                        else
                            hi = mid;
                    }

                    return hi;
                }

        private:
            /* bu_abbrev_v[dim] holds known units for native unit dim */
            std::vector<native_scale_v> bu_abbrev_vv_;
        };

    } /*namespace unit*/
} /*namespace xo*/


/** end basis_unit2.hpp **/
