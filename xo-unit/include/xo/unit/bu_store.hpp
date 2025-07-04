/** @file bu_store.hpp **/

#pragma once

//#include "bpu.hpp"
#include "basis_unit.hpp"
#include "xo/ratio/ratio.hpp"
#include <array>
#include <cstdint>

namespace xo {
    namespace qty {
        using bpu_abbrev_type = flatstring<24>;

        using power_ratio_type = xo::ratio::ratio<std::int64_t>;

        namespace detail {
            /** @class bu_dim_store
             *  @brief store basis-unit abbreviations for a particular dimension
             **/
            struct bu_dim_store {
                /** max number of basis-units per dimension **/
                static constexpr std::size_t max_bu_per_dim = 25;

                /** @defgroup bu-dim-store-type-traits bu-dim-store type traits **/
                ///@{
                using entry_type = std::pair<scalefactor2x_ratio_type, bu_abbrev_type>;

                /* e.g.
                 *   [(1/1000000000, "nm"), (1/1000000, "um"), (1/1000, "mm"), (1/1, "m"), (1000/1, "km")]
                 */
                using native_scale_v = std::array<entry_type, max_bu_per_dim>;
                ///@}

            public:
                constexpr bu_dim_store() = default;

                constexpr bool empty() const { return n_bu_ == 0; }
                constexpr std::size_t size() const { return n_bu_; }

                constexpr const entry_type & operator[](std::size_t i) const { return bu_abbrev_v_[i]; }

                /** @brief get least-upper-bound index position in bu_abbrev_v[]
                 *
                 *  return value in [0, n] where n = .size()
                 **/
                constexpr std::size_t abbrev_lub_ix(const scalefactor_ratio_type & scalefactor) const
                    {
                        if (n_bu_ == 0)
                            return 0;

                        std::size_t lo = 0;
                        std::size_t hi = n_bu_-1;

                        if (scalefactor <= bu_abbrev_v_[lo].first)
                            return 0;

                        auto cmp = (scalefactor <=> bu_abbrev_v_[hi].first);

                        if (cmp > 0)
                            return n_bu_;

                        if (cmp == 0)
                            return hi;

                        while (hi-lo > 1) {
                            /* inv:
                             *   bu_abbrev_v[lo].first < scalefactor <= bu_abbrev_v[hi].first
                             */

                            std::size_t mid = lo + (hi - lo)/2;

                            if (scalefactor > bu_abbrev_v_[mid].first)
                                lo = mid;
                            else
                                hi = mid;
                        }

                        return hi;
                    }

                constexpr void insert_aux(std::size_t ix,
                                          const entry_type & entry)
                    {

                        if (n_bu_ >= max_bu_per_dim)
                            return;

                        ++n_bu_;

                        for (std::size_t dest_ix = n_bu_; dest_ix > ix; --dest_ix)
                            bu_abbrev_v_[dest_ix] = bu_abbrev_v_[dest_ix - 1];

                        bu_abbrev_v_[ix] = entry;
                    }

                /** @brief establish abbreviation @p abbrev for basis unit @p bu
                 **/
                constexpr void bu_establish_abbrev(const scalefactor_ratio_type & scalefactor,
                                                   const bu_abbrev_type & abbrev)
                    {

                        std::int32_t i_abbrev = this->abbrev_lub_ix(scalefactor);

                        auto entry = std::make_pair(scalefactor, abbrev);

                        if ((i_abbrev < bu_abbrev_v_.size())
                            && (bu_abbrev_v_[i_abbrev].first == scalefactor))
                        {
                            bu_abbrev_v_[i_abbrev] = entry;
                        } else {
                            this->insert_aux(i_abbrev, entry);
                        }
                    }

            public:
                /** @defgroup bu-dim-store-instance-vars bu-dim-store instance vars **/
                ///@{
                std::size_t n_bu_ = 0;
                std::array<entry_type, max_bu_per_dim> bu_abbrev_v_;
                ///@}
            }; /*bu_dim_store*/

            /** @class bu_store
             *  @brief associate basis units with abbreviations
             **/
            struct bu_store {
                /** @defgroup bu-store-constructors bu-store constructors **/
                ///@{
                /** construct canonical instance containing all known basis units **/
                constexpr bu_store() {
                    // ----- mass -----

                    this->bu_establish_abbrev(detail::bu::picogram,         bu_abbrev_type::from_chars("pg"));
                    this->bu_establish_abbrev(detail::bu::nanogram,         bu_abbrev_type::from_chars("ng"));
                    this->bu_establish_abbrev(detail::bu::microgram,        bu_abbrev_type::from_chars("ug"));
                    this->bu_establish_abbrev(detail::bu::milligram,        bu_abbrev_type::from_chars("mg"));
                    this->bu_establish_abbrev(detail::bu::gram,             bu_abbrev_type::from_chars("g"));
                    this->bu_establish_abbrev(detail::bu::kilogram,         bu_abbrev_type::from_chars("kg"));
                    this->bu_establish_abbrev(detail::bu::tonne,            bu_abbrev_type::from_chars("t"));
                    this->bu_establish_abbrev(detail::bu::kilotonne,        bu_abbrev_type::from_chars("kt"));
                    this->bu_establish_abbrev(detail::bu::megatonne,        bu_abbrev_type::from_chars("Mt"));
                    this->bu_establish_abbrev(detail::bu::gigatonne,        bu_abbrev_type::from_chars("Gt"));

                    // ----- distance -----

                    this->bu_establish_abbrev(detail::bu::picometer,        bu_abbrev_type::from_chars("pm"));
                    this->bu_establish_abbrev(detail::bu::nanometer,        bu_abbrev_type::from_chars("nm"));
                    this->bu_establish_abbrev(detail::bu::micrometer,       bu_abbrev_type::from_chars("um"));
                    this->bu_establish_abbrev(detail::bu::millimeter,       bu_abbrev_type::from_chars("mm"));
                    this->bu_establish_abbrev(detail::bu::meter,            bu_abbrev_type::from_chars("m"));
                    this->bu_establish_abbrev(detail::bu::kilometer,        bu_abbrev_type::from_chars("km"));
                    this->bu_establish_abbrev(detail::bu::megameter,        bu_abbrev_type::from_chars("Mm"));
                    this->bu_establish_abbrev(detail::bu::gigameter,        bu_abbrev_type::from_chars("Gm"));

                    this->bu_establish_abbrev(detail::bu::lightsecond,      bu_abbrev_type::from_chars("lsec"));
                    this->bu_establish_abbrev(detail::bu::astronomicalunit, bu_abbrev_type::from_chars("AU"));

                    this->bu_establish_abbrev(detail::bu::inch,             bu_abbrev_type::from_chars("in"));
                    this->bu_establish_abbrev(detail::bu::foot,             bu_abbrev_type::from_chars("ft"));
                    this->bu_establish_abbrev(detail::bu::yard,             bu_abbrev_type::from_chars("yd"));
                    this->bu_establish_abbrev(detail::bu::mile,             bu_abbrev_type::from_chars("mi"));

                    // ----- time -----

                    this->bu_establish_abbrev(detail::bu::picosecond,       bu_abbrev_type::from_chars("ps"));
                    this->bu_establish_abbrev(detail::bu::nanosecond,       bu_abbrev_type::from_chars("ns"));
                    this->bu_establish_abbrev(detail::bu::microsecond,      bu_abbrev_type::from_chars("us"));
                    this->bu_establish_abbrev(detail::bu::millisecond,      bu_abbrev_type::from_chars("ms"));
                    this->bu_establish_abbrev(detail::bu::second,           bu_abbrev_type::from_chars("s"));
                    this->bu_establish_abbrev(detail::bu::minute,           bu_abbrev_type::from_chars("min"));
                    this->bu_establish_abbrev(detail::bu::hour,             bu_abbrev_type::from_chars("hr"));
                    this->bu_establish_abbrev(detail::bu::day,              bu_abbrev_type::from_chars("dy"));
                    this->bu_establish_abbrev(detail::bu::week,             bu_abbrev_type::from_chars("wk"));
                    this->bu_establish_abbrev(detail::bu::month,            bu_abbrev_type::from_chars("mo"));
                    this->bu_establish_abbrev(detail::bu::year250,          bu_abbrev_type::from_chars("yr250"));
                    this->bu_establish_abbrev(detail::bu::year,             bu_abbrev_type::from_chars("yr"));
                    this->bu_establish_abbrev(detail::bu::year360,          bu_abbrev_type::from_chars("yr360"));
                    this->bu_establish_abbrev(detail::bu::year365,          bu_abbrev_type::from_chars("yr365"));

                    // ----- misc (currency, price) -----

                    this->bu_establish_abbrev(detail::bu::currency,         bu_abbrev_type::from_chars("ccy"));
                    this->bu_establish_abbrev(detail::bu::price,            bu_abbrev_type::from_chars("px"));
                }
                ///@}

                /** @defgroup bu-store-implementation-methods **/
                ///@{
                /** report fallback abbreviation for a basis unit.
                 *
                 *  Typically unused.  Will be invoked only if a basis unit exists for which
                 *  @ref bu_store::bu_establish_abbrev was not called by bu_store's constructor.
                 *
                 *  Tries to produce something unambiguous,
                 *  while still supplying enough information
                 *  to indicate what's needed to resolve the problem.
                 *
                 *  Example
                 *  @code
                 *  bu_store.bu_fallback_abbrev(dim::mass, scalefactor_ratio_type(1234,1))
                 *    => "1234g"
                 *  @endcode
                 **/
                static constexpr bu_abbrev_type
                bu_fallback_abbrev(dim basis_dim,
                                   const scalefactor_ratio_type & scalefactor)
                    {
                        return (bu_abbrev_type::from_flatstring
                                (flatstring_concat
                                 (scalefactor.to_str<bu_abbrev_type::fixed_capacity>(),
                                  native_unit2_v[static_cast<std::uint32_t>(basis_dim)].abbrev_str())));
                    }
                ///@}

                /** @defgroup bu-store-access-methods **/
                ///@{
                /** @brief get basis-unit abbreviation at runtime **/
                constexpr bu_abbrev_type bu_abbrev(const basis_unit & bu) const
                    {
                        const auto & bu_abbrev_v = bu_abbrev_vv_[static_cast<std::size_t>(bu.native_dim())];

                        std::size_t i_abbrev = bu_abbrev_v.abbrev_lub_ix(bu.scalefactor());

                        if ((i_abbrev < bu_abbrev_v.size())
                            && (bu_abbrev_v[i_abbrev].first == bu.scalefactor()))
                        {
                            return bu_abbrev_v[i_abbrev].second;
                        } else {
                            return bu_fallback_abbrev(bu.native_dim(), bu.scalefactor());
                        }
                    }
                ///@}

                /** @addtogroup bu-store-implementation-methods **/
                ///@{
                /** associate abbreviation @p abbrev with basis unit @p bu
                 *
                 *  Example:
                 *  @code
                 *  // femtograms
                 *  bu_store.bu_establish_abbrev(detail::bu::mass_unit(1, 1000000000000000), "fg")
                 *  @endcode
                 **/
                constexpr void bu_establish_abbrev(const basis_unit & bu,
                                                   const bu_abbrev_type & abbrev) {
                    auto & dim_store = bu_abbrev_vv_[static_cast<std::size_t>(bu.native_dim_)];

                    dim_store.bu_establish_abbrev(bu.scalefactor_, abbrev);
                }
                ///@}

            public: /* ntoe: public members required so bu_dim_store can be a structural type */
                /** @defgroup bu-store-instance-vars **/
                ///@{
                /** bu-store contents, indexed by native dimension **/
                std::array<bu_dim_store, n_dim> bu_abbrev_vv_;
                ///@}
            };
        } /*namespace detail*/

        /** @brief global abbreviation store.
         *
         *  @note
         *  Extending the contents of this store at runtime is not supported,
         *  in favor of preserving constexpr abbreviations.
         **/
        static constexpr detail::bu_store bu_abbrev_store = detail::bu_store();

        /** @brief get abbreviation for basis-unit @p bu **/
        constexpr bu_abbrev_type
        bu_abbrev(const basis_unit & bu)
        {
            return bu_abbrev_store.bu_abbrev(bu);
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end bu_store.hpp **/
