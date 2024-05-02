/* @file scaled_unit.test.cpp */

#include "xo/unit/scaled_unit.hpp"
#include "xo/unit/scaled_unit_iostream.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace qty {
        using su64_type = scaled_unit<std::int64_t>;
        using xo::qty::detail::su_product;
        using xo::qty::detail::nu_maker;

        /* compile-time test:
         * verify we can use an su64_type instance as a non-type template parameter.
         * Will need this for quantity<Repr, Int, scaled_unit<Int>>
         */
        template <su64_type su>
        constexpr su64_type su_reciprocal = su.reciprocal();

        TEST_CASE("scaled_unit", "[scaled_unit]") {
            static_assert(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.natural_unit_ == nu::gram.reciprocal());
            REQUIRE(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.natural_unit_ == nu::gram.reciprocal());

            static_assert(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.outer_scale_factor_ == 1);
            REQUIRE(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.outer_scale_factor_ == 1);

            static_assert(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.outer_scale_sq_ == 1.0);
            REQUIRE(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.outer_scale_sq_ == 1.0);
        } /*TEST_CASE(scaled_unit)*/

        TEST_CASE("su_product", "[scaled_unit]") {
            constexpr bool c_debug_flag = false;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.su_product"));

            {
                constexpr natural_unit<int64_t> v
                    = (nu_maker<int64_t>::make_nu
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);

                constexpr natural_unit<int64_t> w
                    = (nu_maker<int64_t>::make_nu
                       (bpu<int64_t>(dim::time, scalefactor_ratio_type(30*24*3600, 1), power_ratio_type(-1, 2))));

                static_assert(w.n_bpu() == 1);

                constexpr auto prod_rr = su_product<int64_t, __int128_t>(v, w);

                log && log(xtag("prod_rr.bpu_array", prod_rr.natural_unit_));
                log && log(xtag("prod_rr.outer_scale_exact", prod_rr.outer_scale_factor_.convert_to<int64_t>()));
                log && log(xtag("prod_rr.outer_scale_sq", prod_rr.outer_scale_sq_));

                static_assert(prod_rr.natural_unit_.n_bpu() == 3);
                static_assert(prod_rr.natural_unit_[0].native_dim() == dim::distance);
                static_assert(prod_rr.natural_unit_[0].scalefactor() == scalefactor_ratio_type(1, 1000));
                static_assert(prod_rr.natural_unit_[0].power() == power_ratio_type(2, 1));
                static_assert(prod_rr.natural_unit_[1].native_dim() == dim::mass);
                static_assert(prod_rr.natural_unit_[1].scalefactor() == scalefactor_ratio_type(1, 1000));
                static_assert(prod_rr.natural_unit_[1].power() == power_ratio_type(-1, 1));
                static_assert(prod_rr.natural_unit_[2].native_dim() == dim::time);
                static_assert(prod_rr.natural_unit_[2].scalefactor() == scalefactor_ratio_type(30*24*3600, 1));
                static_assert(prod_rr.natural_unit_[2].power() == power_ratio_type(-1, 2));
                static_assert(prod_rr.outer_scale_factor_ == scalefactor_ratio_type(1, 1));
                static_assert(prod_rr.outer_scale_sq_ == 1.0);
            }
        } /*TEST_CASE(su_product)*/

        TEST_CASE("scaled_unit0", "[scaled_unit0]") {
            constexpr bool c_debug_flag = false;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.scaled_unit0"));

            constexpr auto ng = su::nanogram;
            constexpr auto ng2 = ng * ng;

            log && log(xtag("ng", ng));
            log && log(xtag("ng*ng", ng2));

            static_assert(ng.natural_unit_.n_bpu() == 1);
            static_assert(ng2.natural_unit_.n_bpu() == 1);
        } /*TEST_CASE(scaled_unit0)*/

        TEST_CASE("scaled_unit1", "[scaled_unit1]") {
            constexpr bool c_debug_flag = false;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.scaled_unit1"));

            constexpr auto ng = su::nanogram;
            constexpr auto ng2 = ng / ng;

            log && log(xtag("ng", ng));
            log && log(xtag("ng/ng", ng2));

            static_assert(ng.natural_unit_.n_bpu() == 1);
            static_assert(ng2.natural_unit_.n_bpu() == 0);
        } /*TEST_CASE(scaled_unit1)*/

    } /*namespace qty*/
} /*namespace xo*/

/* end scaled_unit.test.cpp */
