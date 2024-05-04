/* @file natural_unit.test.cpp */

#include "xo/unit/scaled_unit.hpp"
#include "xo/unit/scaled_unit_iostream.hpp"
#include "xo/unit/natural_unit.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::qty::detail::su_product;
    using xo::qty::detail::su_ratio;
    using xo::qty::detail::nu_ratio_inplace;
    using xo::qty::detail::nu_maker;
    using xo::qty::detail::bpu2_rescale; // -> nu_rescale or bpu_rescale

    namespace qty {
        using nu64_type = natural_unit<std::int64_t>;

        /* compile-time test:
         * verify we can use an nu64_type instance as a non-type template parameter.
         * Will need this for quantity<Repr, Int, natural_unit<Int>>
         */
        template <nu64_type nu>
        constexpr nu_abbrev_type nu_mpl_abbrev = nu.abbrev();

        TEST_CASE("natural_unit", "[natural_unit]") {
            static_assert(nu_mpl_abbrev<nu::gram> == nu::gram.abbrev());
            REQUIRE(nu_mpl_abbrev<nu::gram> == nu::gram.abbrev());
        } /*TEST_CASE(natural_unit)*/

        TEST_CASE("natural_unit0", "[natural_unit]") {
            constexpr bool c_debug_flag = false;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.natural_unit0"));

            {
                constexpr natural_unit<int64_t> v
                    = (nu_maker<int64_t>::make_nu
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);

                log && log(xtag("v.abbrev", v.abbrev()));

                static_assert(v.abbrev().size() > 0);
                static_assert(v.abbrev() == flatstring("mm^2.mg^-1"));
            }
        } /*TEST_CASE(natural_unit0)*/

        TEST_CASE("natural_unit1", "[natural_unit]") {
            constexpr bool c_debug_flag = false;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.natural_unit1"));

            {
                constexpr natural_unit<int64_t> v
                    = (nu_maker<int64_t>::make_nu
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1000, 1), power_ratio_type(2, 1))));

                static_assert(v.n_bpu() == 1);

                log && log(xtag("v.abbrev", v.abbrev()));

                static_assert(v.abbrev().size() > 0);
                static_assert(v.abbrev() == flatstring("km^2"));
            }
        } /*TEST_CASE(natural_unit1)*/

        TEST_CASE("natural_unit2", "[natural_unit]") {
            constexpr bool c_debug_flag = false;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.natural_unit2"));

            {
                constexpr natural_unit<int64_t> v
                    = (nu_maker<int64_t>::make_nu
                       (bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1)),
                        bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1), power_ratio_type(1, 1)),
                        bpu<int64_t>(dim::time, scalefactor_ratio_type(1, 1), power_ratio_type(-2, 1))));

                static_assert(v.n_bpu() == 3);

                log && log(xtag("v.abbrev", v.abbrev()));

                static_assert(v.abbrev().size() > 0);
                static_assert(v.abbrev() == flatstring("kg.m.s^-2"));
            }
        } /*TEST_CASE(natural_unit2)*/

        TEST_CASE("natural_unit3", "[natural_unit]") {
            constexpr bool c_debug_flag = false;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.natural_unit3"));

            {
                constexpr natural_unit<int64_t> v
                    = (nu_maker<int64_t>::make_nu
                       (bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1)),
                        bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1), power_ratio_type(1, 1))));

                static_assert(v.n_bpu() == 2);

                log && log(xtag("v.abbrev", v.abbrev()));

                static_assert(v.abbrev().size() > 0);
                static_assert(v.abbrev() == flatstring("kg.m"));

                {
                    natural_unit<int64_t> w = v;

                    nu_ratio_inplace(&w,
                                     bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1)));

                    log && log(xtag("w.abbrev", w.abbrev()));

                    REQUIRE(w.n_bpu() == 1);
                    REQUIRE(w[0].native_dim() == dim::distance);
                    REQUIRE(w.abbrev() == flatstring("m"));
                }

                {
                    constexpr natural_unit<int64_t> w
                        = (nu_maker<int64_t>::make_nu
                           (bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1))));

                    static_assert(w.n_bpu() == 1);

                    log && log(xtag("w.abbrev", w.abbrev()));

                    constexpr auto rr = su_ratio<int64_t, __int128_t>(v, w);

                    log && log(xtag("rr", rr));

                    REQUIRE(rr.natural_unit_.n_bpu() == 1);
                    REQUIRE(rr.natural_unit_[0].native_dim() == dim::distance);
                    REQUIRE(rr.natural_unit_.abbrev() == flatstring("m"));
                }

                {
                    constexpr natural_unit<int64_t> w
                        = (nu_maker<int64_t>::make_nu
                           (bpu<int64_t>(dim::time, scalefactor_ratio_type(1, 1), power_ratio_type(1, 1))));

                    static_assert(w.n_bpu() == 1);

                    log && log(xtag("w.abbrev", w.abbrev()));

                    constexpr auto rr = su_ratio<int64_t, __int128_t>(v, w);

                    log && log(xtag("rr", rr));

                    REQUIRE(rr.natural_unit_.n_bpu() == 3);
                    REQUIRE(rr.natural_unit_[0].native_dim() == dim::mass);
                    REQUIRE(rr.natural_unit_[1].native_dim() == dim::distance);
                    REQUIRE(rr.natural_unit_[2].native_dim() == dim::time);
                    REQUIRE(rr.natural_unit_.abbrev() == flatstring("kg.m.s^-1"));
                }

                {
                    natural_unit<int64_t> w = v;

                    REQUIRE(w.n_bpu() == 2);
                    REQUIRE(w[0].native_dim() == dim::mass);

                    nu_ratio_inplace(&w,
                                     bpu<int64_t>(dim::time, scalefactor_ratio_type(1, 1), power_ratio_type(2, 1)));

                    REQUIRE(w.n_bpu() == 3);
                    REQUIRE(w[0].native_dim() == dim::mass);
                    REQUIRE(w[1].native_dim() == dim::distance);
                    REQUIRE(w[2].native_dim() == dim::time);

                    log && log(xtag("w.abbrev", w.abbrev()));

                    REQUIRE(w.n_bpu() == 3);
                    REQUIRE(w.abbrev() == flatstring("kg.m.s^-2"));
                }
            }
        } /*TEST_CASE(natural_unit3)*/

        TEST_CASE("bpu_rescale", "[bpu_rescale]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_rescale"));
            //log && log("(A)", xtag("foo", foo));

            /* keep spelled-out test.  Will generalize to fractional powers when c++26 available */
            {
                constexpr auto p = power_ratio_type(1, 1);

                constexpr auto orig_bpu = bpu<int64_t>(dim::mass,
                                                        scalefactor_ratio_type(1000, 1),
                                                        power_ratio_type(1, 1));
                static_assert(orig_bpu.native_dim() == dim::mass);

                constexpr auto new_scalefactor = scalefactor_ratio_type(1000000, 1);

                constexpr auto mult = orig_bpu.scalefactor() / new_scalefactor;
                static_assert(mult.num() == 1);
                static_assert(mult.den() == 1000);

                constexpr auto p_floor = orig_bpu.power().floor();
                static_assert(p_floor == 1);

                constexpr auto p_frac = orig_bpu.power().frac().template convert_to<double>();
                static_assert(p_frac == 0.0);

                constexpr auto outer_sf_exact = mult.power(p_floor);
                static_assert(outer_sf_exact.num() == 1);
                static_assert(outer_sf_exact.den() == 1000);

                constexpr auto mult_inexact = mult.template convert_to<double>();
                static_assert(mult_inexact == 0.001);

                constexpr auto rr = bpu2_rescale<int64_t>(orig_bpu, scalefactor_ratio_type(1000000, 1));

                static_assert(rr.bpu_rescaled_.power() == power_ratio_type(1,1));
                static_assert(rr.outer_scale_factor_ == outer_sf_exact);
                static_assert(rr.outer_scale_sq_ == 1.0);
            }

            /* keep spelled-out test.  Will generalize to other fractional powers when c++26 available */
            {
                constexpr auto p = power_ratio_type(-1, 2);

                constexpr auto orig_bpu = bpu<int64_t>(dim::time,
                                                       scalefactor_ratio_type(360*24*3600, 1),
                                                       power_ratio_type(-1, 2));
                static_assert(orig_bpu.native_dim() == dim::time);

                constexpr auto new_scalefactor = scalefactor_ratio_type(30*24*3600, 1);

                /* orig ~ 360d volatility,  new = 30d volatility */
                constexpr auto mult = orig_bpu.scalefactor() / new_scalefactor;
                log && log(xtag("mult", mult));
                static_assert(mult.num() == 12);
                static_assert(mult.den() == 1);

                constexpr auto p_floor = orig_bpu.power().floor();
                static_assert(p_floor == 0);

                constexpr auto p_frac = orig_bpu.power().frac().template convert_to<double>();
                static_assert(p_frac == -0.5);

                constexpr auto outer_sf_exact = mult.power(p_floor);
                static_assert(outer_sf_exact.num() == 1);
                static_assert(outer_sf_exact.den() == 1);

                constexpr auto mult_inexact = mult.template convert_to<double>();
                static_assert(mult_inexact == 12.0);

                constexpr auto rr = bpu2_rescale<int64_t>(orig_bpu, scalefactor_ratio_type(30*24*3600, 1));

                log && log(xtag("rr.outer_scale_exact", rr.outer_scale_factor_),
                           xtag("rr.outer_scale_sq", rr.outer_scale_sq_));

                static_assert(rr.bpu_rescaled_.power() == power_ratio_type(-1,2));
                static_assert(rr.outer_scale_factor_ == outer_sf_exact);
                static_assert(rr.outer_scale_sq_ == 1 / 12.0);
            }

            /* keep spelled-out test.  Will generalize to other fractional powers when c++26 available */
            {
                constexpr auto p = power_ratio_type(-3, 2);

                constexpr auto orig_bpu = bpu<int64_t>(dim::time,
                                                        scalefactor_ratio_type(360*24*3600, 1),
                                                        power_ratio_type(-3, 2));
                static_assert(orig_bpu.native_dim() == dim::time);

                constexpr auto new_scalefactor = scalefactor_ratio_type(30*24*3600, 1);

                constexpr auto mult = orig_bpu.scalefactor() / new_scalefactor;
                log && log(xtag("mult", mult));
                static_assert(mult.num() == 12);
                static_assert(mult.den() == 1);

                constexpr auto p_floor = orig_bpu.power().floor();
                static_assert(p_floor == -1);

                constexpr auto p_frac = orig_bpu.power().frac().template convert_to<double>();
                static_assert(p_frac == -0.5);

                constexpr auto outer_sf_exact = mult.power(p_floor);
                static_assert(outer_sf_exact.num() == 1);
                static_assert(outer_sf_exact.den() == 12);

                constexpr auto mult_inexact = mult.template convert_to<double>();
                static_assert(mult_inexact == 12.0);

                constexpr auto rr = bpu2_rescale<int64_t>(orig_bpu, scalefactor_ratio_type(30*24*3600, 1));

                log && log(xtag("rr.outer_scale_exact", rr.outer_scale_factor_),
                           xtag("rr.outer_scale_sq", rr.outer_scale_sq_));

                static_assert(rr.bpu_rescaled_.power() == power_ratio_type(-3,2));
                static_assert(rr.outer_scale_factor_ == outer_sf_exact);
                static_assert(rr.outer_scale_sq_ == 1 / 12.0);
            }
        } /*TEST_CASE(bpu_rescale)*/

        TEST_CASE("bpu_product", "[bpu_product]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_product"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr auto bpu_x = bpu<int64_t>(dim::time,
                                                     scalefactor_ratio_type(360*24*3600, 1),
                                                     power_ratio_type(-3,2));
                static_assert(bpu_x.native_dim() == dim::time);

                constexpr auto bpu_y = bpu<int64_t>(dim::time,
                                                     scalefactor_ratio_type(360*24*3600, 1),
                                                     power_ratio_type(1,2));
                static_assert(bpu_y.native_dim() == dim::time);

#ifdef NOT_USING
                constexpr auto bpu_prod = bpu2_product<int64_t>(bpu_x, bpu_y);

                log && log(xtag("bpu_prod.bpu_rescaled", bpu_prod.bpu_rescaled_));
                log && log(xtag("bpu_prod.outer_scale_exact", bpu_prod.outer_scale_exact_));
                log && log(xtag("bpu_prod.outer_scale_sq", bpu_prod.outer_scale_sq_));

                static_assert(bpu_prod.bpu_rescaled_.native_dim() == dim::time);
                static_assert(bpu_prod.bpu_rescaled_.scalefactor() == scalefactor_ratio_type(360*24*3600, 1));
                static_assert(bpu_prod.bpu_rescaled_.power() == power_ratio_type(-1, 1));
                static_assert(bpu_prod.outer_scale_exact_ == scalefactor_ratio_type(1,1));
                static_assert(bpu_prod.outer_scale_sq_ == 1.0);
#endif
            }
        } /*TEST_CASE(bpu_product)*/

        TEST_CASE("bpu_product2", "[bpu_product]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_product2"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr auto bpu_x = bpu<int64_t>(dim::time,
                                                     scalefactor_ratio_type(360*24*3600, 1),
                                                     power_ratio_type(-3,2));
                static_assert(bpu_x.native_dim() == dim::time);

                constexpr auto bpu_y = bpu<int64_t>(dim::time,
                                                     scalefactor_ratio_type(30*24*3600, 1),
                                                     power_ratio_type(1,2));
                static_assert(bpu_y.native_dim() == dim::time);

#ifdef NOT_USING
                constexpr auto bpu_prod = bpu2_product<int64_t>(bpu_x, bpu_y);

                log && log(xtag("bpu_prod.bpu_rescaled", bpu_prod.bpu_rescaled_));
                log && log(xtag("bpu_prod.outer_scale_exact", bpu_prod.outer_scale_exact_));
                log && log(xtag("bpu_prod.outer_scale_sq", bpu_prod.outer_scale_sq_));

                static_assert(bpu_prod.bpu_rescaled_.native_dim() == dim::time);
                static_assert(bpu_prod.bpu_rescaled_.scalefactor() == scalefactor_ratio_type(360*24*3600, 1));
                static_assert(bpu_prod.bpu_rescaled_.power() == power_ratio_type(-1, 1));
                static_assert(bpu_prod.outer_scale_exact_ == scalefactor_ratio_type(1,1));
                static_assert(bpu_prod.outer_scale_sq_ == 1.0/12.0);
#endif
            }
        } /*TEST_CASE(bpu_product2)*/

        TEST_CASE("bpu_array", "[bpu_array]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_array"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr natural_unit<int64_t> v;

                static_assert(v.n_bpu() == 0);
            }

            {
                constexpr natural_unit<int64_t> v
                    = (nu_maker<int64_t>::make_nu
                       (bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1))));

                static_assert(v.n_bpu() == 1);
            }

            {
                constexpr natural_unit<int64_t> v
                    = (nu_maker<int64_t>::make_nu
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);
            }
        } /*TEST_CASE(bpu_array)*/
    } /*namespace qty*/
} /*namespace xo*/


/* end natural_unit.test.cpp */
