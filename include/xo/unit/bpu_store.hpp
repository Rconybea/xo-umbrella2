/** @file bpu_store.hpp **/

#pragma once

#include "bpu.hpp"

namespace xo {
    namespace qty {
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
                                              const scalefactor_ratio_type & scalefactor) const
                {
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

            /** @brief get basis-power-unit abbreviation at runtime **/
            bpu2_abbrev_type bpu_abbrev(dim basis_dim,
                                        const scalefactor_ratio_type & scalefactor,
                                        const power_ratio_type & power)
                {
                    return abbrev::bpu2_abbrev(basis_dim,
                                               scalefactor,
                                               power);
                }

            template <dim BasisDim, std::int64_t InnerScaleNum, std::int64_t InnerScaleDen>
            void bu_establish_abbrev_for() {
                this->bu_establish_abbrev
                    (basis_unit(BasisDim,
                                 scalefactor_ratio_type(InnerScaleNum, InnerScaleDen)),
                     units::scaled_native_unit2_abbrev_v<BasisDim, InnerScaleNum, InnerScaleDen>);
            }

            /** @brief establish abbreviation @p abbrev for basis unit @p bu
             **/
            void bu_establish_abbrev(const basis_unit & bu,
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
    } /*namespace qty*/
} /*namespace xo*/

/** end bpu_store.hpp **/
