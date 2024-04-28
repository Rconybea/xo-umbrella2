/* @file bpu.test.cpp */

#include "xo/unit/bpu.hpp"
#include "xo/indentlog/scope.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::qty::abbrev::bpu_abbrev;
    using xo::qty::abbrev::flatstring_from_exponent;

    namespace qty {
        using bpu64_type = bpu<std::int64_t>;

        /* compile-time test:
         * verify we can use a bpu64_type instance as a non-type template parameter.
         * Will need this for quantity<Repr, Int, natural_unit<Int>>
         */
        template <bpu64_type bpu>
        constexpr bpu_abbrev_type bpu_mpl_abbrev = bpu.abbrev();

        TEST_CASE("bpu", "[bpu]") {
            //constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            //scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu"));
            //log && log("(A)", xtag("foo", foo));

            static_assert(bpu_mpl_abbrev<bpu64_type::unit_power(bu::gram)> == bpu64_type::unit_power(bu::gram).abbrev());
            REQUIRE(bpu_mpl_abbrev<bpu64_type::unit_power(bu::gram)> == bpu64_type::unit_power(bu::gram).abbrev());
        } /*TEST_CASE(bpu)*/

        TEST_CASE("flatstring_from_exponent", "[flatstring_from_exponent]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.flatstring_from_exponent"));
            //log && log("(A)", xtag("foo", foo));

            log && log(xtag("^-3", flatstring_from_exponent(-3,1)));
            log && log(xtag("^-2", flatstring_from_exponent(-2,1)));
            log && log(xtag("^-1", flatstring_from_exponent(-1,1)));
            log && log(xtag("^-1/2", flatstring_from_exponent(-1,2)));
            log && log(xtag("^0", flatstring_from_exponent(0,1)));
            log && log(xtag("^1/2", flatstring_from_exponent(1,2)));
            log && log(xtag("^1", flatstring_from_exponent(1,1)));
            log && log(xtag("^2", flatstring_from_exponent(2,1)));
            log && log(xtag("^3", flatstring_from_exponent(3,1)));

            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(-3,1))
                          == flatstring<5>::from_flatstring(flatstring("^-3")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(-2,1))
                          == flatstring<5>::from_flatstring(flatstring("^-2")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(-1,1))
                          == flatstring<5>::from_flatstring(flatstring("^-1")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(-1,2))
                          == flatstring<5>::from_flatstring(flatstring("^(-1/2)")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(1,2))
                          == flatstring<5>::from_flatstring(flatstring("^(1/2)")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(1,1))
                          == flatstring<5>::from_flatstring(flatstring("")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(2,1))
                          == flatstring<5>::from_flatstring(flatstring("^2")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(3,1))
                          == flatstring<5>::from_flatstring(flatstring("^3")));
        } /*TEST_CASE(flatstring_from_exponent)*/

        TEST_CASE("bpu_abbrev", "[bpu_abbrev]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu2_assemble_abbrev"));
            //log && log("(A)", xtag("foo", foo));

            log && log(xtag("1/(kg*kg)", bpu_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-2, 1))));
            log && log(xtag("1/kg", bpu_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-1, 1))));
            log && log(xtag("kg", bpu_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1))));
            log && log(xtag("kg*kg", bpu_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(2, 1))));

            static_assert(bpu<int64_t>(dim::mass, scalefactor_ratio_type(1, 1), power_ratio_type(1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("g"));
            static_assert(bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("kg"));
            static_assert(bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("kg^-1"));
            static_assert(bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-2, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("kg^-2"));

            static_assert(bpu<int64_t>(dim::time, scalefactor_ratio_type(60, 1), power_ratio_type(-2, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("min^-2"));
            static_assert(bpu<int64_t>(dim::time, scalefactor_ratio_type(3600, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("hr^-1"));
            static_assert(bpu<int64_t>(dim::time, scalefactor_ratio_type(24*3600, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("dy^-1"));
            static_assert(bpu<int64_t>(dim::time, scalefactor_ratio_type(360*24*3600, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("yr360^-1"));
            static_assert(bpu<int64_t>(dim::time, scalefactor_ratio_type(360*24*3600, 1), power_ratio_type(-1, 2)).abbrev()
                          == bpu_abbrev_type::from_chars("yr360^(-1/2)"));
        } /*TEST_CASE(bpu_abbrev)*/

    } /*namespace qty*/
} /*namespace xo*/

/* end bpu.test.cpp */
