/** @file gaussianpair.test.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "xo/randomgen/gaussianpairgen.hpp"
#include "xo/randomgen/xoshiro256.hpp"
#include <catch2/catch.hpp>
#include <cmath>

namespace ut {
    using xo::rng::gaussianpair_dist;
    using xo::rng::gaussianpairgen;
    using xo::rng::xoshiro256ss;

    namespace {
        constexpr std::uint64_t c_seed = 14950319842636922572UL;

        /* enough samples that the sample correlation is tight around rho:
         * se(corr) ~ (1-rho^2)/sqrt(n) ~ 0.003 at n=1e5, so the 0.02
         * tolerances below have ample margin.  NB the generator is seeded,
         * so this test is deterministic -- the tolerance absorbs a future
         * change of sequence, not run-to-run noise.
         */
        constexpr std::size_t c_n_sample = 100000;

        struct pair_stats {
            double mean1 = 0.0, mean2 = 0.0;
            double var1 = 0.0, var2 = 0.0;
            double corr = 0.0;
        };

        /* sample n pairs at correlation rho, and reduce to moments */
        pair_stats
        sample_stats(double rho, std::size_t n) {
            auto gen = gaussianpairgen<xoshiro256ss>::make(xoshiro256ss(c_seed),
                                                           rho);

            double s1 = 0.0, s2 = 0.0, s11 = 0.0, s22 = 0.0, s12 = 0.0;

            for (std::size_t i = 0; i < n; ++i) {
                auto yy = gen();

                s1  += yy[0];
                s2  += yy[1];
                s11 += yy[0] * yy[0];
                s22 += yy[1] * yy[1];
                s12 += yy[0] * yy[1];
            }

            double dn = static_cast<double>(n);

            pair_stats retval;

            retval.mean1 = s1 / dn;
            retval.mean2 = s2 / dn;
            retval.var1 = (s11 / dn) - (retval.mean1 * retval.mean1);
            retval.var2 = (s22 / dn) - (retval.mean2 * retval.mean2);

            double cov = (s12 / dn) - (retval.mean1 * retval.mean2);

            retval.corr = cov / std::sqrt(retval.var1 * retval.var2);

            return retval;
        }
    }

    /* the distribution must satisfy the same concept as any other,
     * since generator<> requires it
     */
    static_assert(xo::rng::distribution_concept<gaussianpair_dist<double>,
                                                xoshiro256ss>);

    TEST_CASE("gaussianpair-moments", "[gaussianpair]") {
        /* Y1, Y2 are N(0,1) whatever rho is; only their correlation changes */
        for (double rho : { -1.0, -0.5, 0.0, 0.25, 0.5, 1.0 }) {
            INFO("rho=" << rho);

            pair_stats st = sample_stats(rho, c_n_sample);

            CHECK(st.mean1 == Approx(0.0).margin(0.02));
            CHECK(st.mean2 == Approx(0.0).margin(0.02));
            CHECK(st.var1 == Approx(1.0).margin(0.02));
            CHECK(st.var2 == Approx(1.0).margin(0.02));
            CHECK(st.corr == Approx(rho).margin(0.02));
        }
    }

    /* rho=+/-1 is the degenerate case: q = sqrt(1-rho^2) = 0, so the second
     * variate drops out and Y2 is exactly +/-Y1 -- an equality, not an
     * approximation
     */
    TEST_CASE("gaussianpair-degenerate", "[gaussianpair]") {
        SECTION("rho=+1") {
            auto gen = gaussianpairgen<xoshiro256ss>::make(xoshiro256ss(c_seed),
                                                           1.0);
            for (std::size_t i = 0; i < 100; ++i) {
                auto yy = gen();
                REQUIRE(yy[1] == yy[0]);
            }
        }

        SECTION("rho=-1") {
            auto gen = gaussianpairgen<xoshiro256ss>::make(xoshiro256ss(c_seed),
                                                           -1.0);
            for (std::size_t i = 0; i < 100; ++i) {
                auto yy = gen();
                REQUIRE(yy[1] == -yy[0]);
            }
        }
    }

    /* same seed -> same sequence.  This is the property that makes a pinned
     * baseline (e.g. xo-kalmanfilter/utest/utestdata/) meaningful, and the
     * reason gaussianpair_dist draws from xo::rng::normal_dist rather than
     * std::normal_distribution -- see normal_dist.hpp
     */
    TEST_CASE("gaussianpair-reproducible", "[gaussianpair]") {
        auto gen1 = gaussianpairgen<xoshiro256ss>::make(xoshiro256ss(c_seed), 0.5);
        auto gen2 = gaussianpairgen<xoshiro256ss>::make(xoshiro256ss(c_seed), 0.5);

        for (std::size_t i = 0; i < 100; ++i) {
            auto a = gen1();
            auto b = gen2();

            REQUIRE(a[0] == b[0]);
            REQUIRE(a[1] == b[1]);
        }
    }

    /* different seeds must not give the same stream */
    TEST_CASE("gaussianpair-seed-sensitive", "[gaussianpair]") {
        auto gen1 = gaussianpairgen<xoshiro256ss>::make(xoshiro256ss(c_seed), 0.5);
        auto gen2 = gaussianpairgen<xoshiro256ss>::make(xoshiro256ss(c_seed + 1), 0.5);

        std::size_t n_same = 0;

        for (std::size_t i = 0; i < 100; ++i) {
            auto a = gen1();
            auto b = gen2();

            if (a[0] == b[0])
                ++n_same;
        }

        CHECK(n_same == 0);
    }

    TEST_CASE("gaussianpair-param", "[gaussianpair]") {
        gaussianpair_dist<double> dist(0.5);

        CHECK(dist.rho() == 0.5);

        /* param() round-trips */
        gaussianpair_dist<double> other;
        other.param(dist.param());

        CHECK(other.param() == dist.param());
        CHECK(other.rho() == 0.5);

        /* default is uncorrelated */
        CHECK(gaussianpair_dist<double>().rho() == 0.0);

        /* r^2 + q^2 = 1, the invariant the construction relies on */
        for (double rho : { -1.0, -0.5, 0.0, 0.25, 1.0 }) {
            auto p = gaussianpair_dist<double>::param_type(rho);

            CHECK(p.r() * p.r() + p.q() * p.q() == Approx(1.0));
        }
    }
} /*namespace ut*/

/* end gaussianpair.test.cpp */
