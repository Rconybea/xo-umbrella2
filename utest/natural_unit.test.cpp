/* @file natural_unit.test.cpp */

#include "xo/unit/scaled_unit.hpp"
#include "xo/unit/scaled_unit_iostream.hpp"
#include "xo/unit/natural_unit.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::qty::detail::su_ratio;
    using xo::qty::detail::nu_ratio_inplace;
    using xo::qty::detail::nu_maker;

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

    } /*namespace qty*/
} /*namespace xo*/


/* end natural_unit.test.cpp */
