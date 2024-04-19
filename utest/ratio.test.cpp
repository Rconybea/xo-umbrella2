/** @file ratio.utest.cpp **/

#include "xo/ratio/ratio.hpp"
#include "xo/ratio/ratio_iostream.hpp"
#include "xo/randomgen/random_seed.hpp"
#include "xo/randomgen/xoshiro256.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/vector.hpp"
#include "xo/indentlog/print/array.hpp"
#include "xo/indentlog/print/tag.hpp"
//#include "xo/indentlog/print/hex.hpp"
#include <catch2/catch.hpp>
#include <random>
#include <numeric>

namespace xo {

    using std::exponential_distribution;
    using std::bernoulli_distribution;

    namespace ut {
        template <typename Int>
        struct ratio_distribution {
            ratio_distribution(double sign_prob, double int_lambda)
                : sign_dist_{sign_prob}, int_dist_{int_lambda} {}

            template <typename Rng>
            xo::ratio::ratio<Int>
            random_ratio(Rng & rng) {
                Int num_sign = sign_dist_(rng) ? -1 : +1;
                Int num = num_sign * (1 + int_dist_(rng));
                Int den_sign = sign_dist_(rng) ? -1 : +1;
                Int den = den_sign * (1 + int_dist_(rng));

                return xo::ratio::ratio(num, den).normalize();
            }

            template <typename Rng>
            xo::ratio::ratio<Int> operator()(Rng & rng) {
                return random_ratio(rng);
            }

            /* generate negative numbers some of the time */
            bernoulli_distribution sign_dist_;
            /* create ratios involving integers,  but don't need integers to be too large */
            exponential_distribution<double> int_dist_;
        };

        template <typename Rng>
        void
        ratio_tests(Rng & rng)
        {
            constexpr bool debug_flag = true;

            std::size_t n_ratio = 25;
            std::size_t n_experiment = n_ratio * n_ratio / 4;
            /* want to avoid integer overflow when exponentiating */
            constexpr int max_pwr = 5;

            scope log(XO_DEBUG2(debug_flag, "ratio_tests"));
            log && log(xtag("n_ratio", n_ratio));

            ratio_distribution<int> ratio_dist(0.25 /*sign_prob*/,
                                               0.05 /*lambda */);
            bernoulli_distribution sign_dist(0.5);
            exponential_distribution<double> power_dist(0.2 /*lambda*/);

            std::vector<xo::ratio::ratio<int>> ratio_v;

            /* ensure 0, 1, -1 all present */
            ratio_v.push_back(xo::ratio::ratio<int>(0,1));
            ratio_v.push_back(xo::ratio::ratio<int>(1,1));
            ratio_v.push_back(xo::ratio::ratio<int>(-1,1));

            for (std::uint32_t i=0, n=n_ratio - ratio_v.size(); i<n; ++i) {
                ratio_v.push_back(ratio_dist(rng));

                REQUIRE(std::gcd(ratio_v[i].num(), ratio_v[i].den()) == 1);
            }

            INFO(XTAG(ratio_v));

            for (std::uint32_t i=0; i<n_experiment; ++i) {
                INFO(tostr(XTAG(i), XTAG(n_experiment)));

                /* choose a couple of ratios at random */
                auto ratio1 = ratio_v[rng() % n_ratio];
                auto ratio2 = ratio_v[rng() % n_ratio];

                double ratio1_approx = ratio1.num() / static_cast<double>(ratio1.den());
                double ratio2_approx = ratio2.num() / static_cast<double>(ratio2.den());

                {
                    auto sum = ratio1 + ratio2;

                    double sum_approx = sum.num() / static_cast<double>(sum.den());

                    log && log(XTAG(ratio1), XTAG(ratio2), XTAG(sum));

                    REQUIRE(sum_approx == Approx(ratio1_approx + ratio2_approx).epsilon(1e-6));
                    REQUIRE(std::gcd(sum.num(), sum.den()) == 1);
                    REQUIRE(sum.den() > 0);

                    /* comparison tests.  piggyback on sum */
                    {
                        auto cmp_approx = (sum_approx <=> ratio1_approx);
                        REQUIRE(cmp_approx == (sum <=> ratio1));
                    }
                    {
                        bool eq = (sum == ratio1);
                        bool eq_approx = (sum_approx == ratio1_approx);
                        REQUIRE(eq == eq_approx);
                    }
                    {
                        bool ne = (sum != ratio1);
                        bool ne_approx = (sum_approx != ratio1_approx);
                        REQUIRE(ne == ne_approx);
                    }
                    {
                        bool gt = (sum > ratio1);
                        bool gt_approx = (sum_approx > ratio1_approx);
                        REQUIRE(gt == gt_approx);
                    }
                    {
                        bool ge = (sum >= ratio1);
                        bool ge_approx = (sum_approx >= ratio1_approx);
                        REQUIRE(ge == ge_approx);
                    }
                    {
                        bool lt = (sum > ratio1);
                        bool lt_approx = (sum_approx > ratio1_approx);
                        REQUIRE(lt == lt_approx);
                    }
                    {
                        bool le = (sum >= ratio1);
                        bool le_approx = (sum_approx >= ratio1_approx);
                        REQUIRE(le == le_approx);
                    }
                }

                {
                    auto neg = -ratio1;

                    double neg_approx = neg.num() / static_cast<double>(neg.den());

                    log && log(XTAG(ratio1), XTAG(neg));

                    REQUIRE(neg_approx == Approx(-ratio1_approx).epsilon(1e-06));
                    REQUIRE(std::gcd(neg.num(), neg.den()) == 1);
                    REQUIRE(neg.den() > 0);
                }

                {
                    auto diff = ratio1 - ratio2;

                    double diff_approx = diff.num() / static_cast<double>(diff.den());

                    log && log(XTAG(ratio1), XTAG(ratio2), XTAG(diff));

                    REQUIRE(diff_approx == Approx(ratio1_approx - ratio2_approx).epsilon(1e-6));
                    REQUIRE(std::gcd(diff.num(), diff.den()) == 1);
                    REQUIRE(diff.den() > 0);
                }

                {
                    auto prod = ratio1 * ratio2;

                    double prod_approx = prod.num() / static_cast<double>(prod.den());

                    log && log(XTAG(ratio1), XTAG(ratio2), XTAG(prod));

                    REQUIRE(prod_approx == Approx(ratio1_approx * ratio2_approx).epsilon(1e-6));
                    REQUIRE(std::gcd(prod.num(), prod.den()) == 1);
                    REQUIRE(prod.den() > 0);
                }

                {
                    auto div = ratio1 * ratio2;

                    double div_approx = div.num() / static_cast<double>(div.den());

                    log && log(XTAG(ratio1), XTAG(ratio2), XTAG(div));

                    REQUIRE(div_approx == Approx(ratio1_approx * ratio2_approx).epsilon(1e-6));
                    REQUIRE(std::gcd(div.num(), div.den()) == 1);
                    REQUIRE(div.den() > 0);
                }

                {
                    int exp = (sign_dist(rng) ? -1 : +1) * power_dist(rng);

                    if (std::abs(exp) >= max_pwr) {
                        exp = (std::signbit(exp) ? -1 : +1) * max_pwr;
                    }

                    auto pwr = ratio1.power(exp);

                    double pwr_approx = pwr.num() / static_cast<double>(pwr.den());

                    log && log(XTAG(ratio1), XTAG(exp), XTAG(pwr));

                    REQUIRE(pwr_approx == Approx(::pow(ratio1_approx, exp)).epsilon(1e-6));
                    REQUIRE(std::gcd(pwr.num(), pwr.den()) == 1);
                    REQUIRE(pwr.den() >= 0);
                }

                {
                    auto ratio1_str = ratio1.template to_str<20>();

                    log && log(XTAG(ratio1_str));
                }
            }

        }

        TEST_CASE("ratio", "[ratio]") {
            //constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            uint64_t seed = 5521646833469436535ul;
            //rng::Seed<rng::xoshiro256ss> seed;

            //std::cerr << "ratio: seed=" << seed << std::endl;

            auto rng = rng::xoshiro256ss(seed);

            ratio_tests(rng);
        } /*TEST_CASE(ratio)*/

    } /*namespace ut*/

} /*namespace xo*/


/** end ratio.utest.cpp **/
