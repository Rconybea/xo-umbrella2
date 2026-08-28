/* @file gaussianpairgen.hpp */

#pragma once

#include "generator.hpp"
#include "normal_dist.hpp"
#include <limits>
#include <array>
#include <cmath>

namespace xo {
    namespace rng {
        /* editor bait: 2d normal, normal xy
         *
         * if
         *   N1 ~ N(0,1)
         *   N2 ~ N(0,1)
         * are two indepenent, normally-distributed r.v's with
         * mean 0 and variance 1,  then
         * let
         *   A = | 1   0 |   X = | N1 |
         *       | r   q |       | N2 |
         *
         * with r^2 + q^2 = 1
         *
         * and consider
         *   A.X = |      N1     | := | Y1 |
         *         | r.N1 + q.N2 |    | Y2 |
         *
         *   Y1, Y2 both have mean 0,
         * since both are linear combination of 0-mean N(0,1) variables
         *
         *   Var(Y1) = 1
         *   Var(Y2) = r^2.Var(N1) + q^2.Var(N2)
         *           = r^2 + q^2
         *           = 1
         *
         * (since N1,N2 indept, and Var(N1)=Var(N2)=1)
         *
         *   Cov(Y1,Y2) = r.Cov(N1,N1) + q.Cov(N1,N2)
         *              = r.Var(N1)
         *              = r
         *
         * (since Cov(N1,N2)=0)
         *
         * we have correlation coefficient for Y1,Y2:
         *
         *                Cov(Y1,Y2)
         * p(Y1,Y2) = --------------------
         *            sqrt(Var(Y1).Var(Y2))
         *
         *          = r
         */
        template<typename FloatType>
        class gaussianpair_dist {
        public:
            using result_type = std::array<FloatType, 2>;

            class param_type {
            public:
                using distribution_type = gaussianpair_dist<FloatType>;

                param_type() = default;
                explicit param_type(FloatType rho)
                    : r_{rho}, q_{std::sqrt(1.0 - rho*rho)} {}

                FloatType rho() const { return r_; }
                FloatType r() const { return r_; }
                FloatType q() const { return q_; }

                bool operator==(const param_type & x) const = default;

            private:
                /* correlation coefficient r */
                FloatType r_ = 0.0;
                /* q := sqrt(1-r^2) */
                FloatType q_ = 1.0;
            }; /*param_type*/

        public:
            gaussianpair_dist() = default;
            /* generate pairs of gaussian N(0,1) random numbers,
             * with correlation coefficient rho
             *
             * Require:
             * - rho in the interval [-1, +1]
             */
            explicit gaussianpair_dist(FloatType rho) : param_{rho} {}
            explicit gaussianpair_dist(const param_type & p) : param_{p} {}

            FloatType rho() const { return param_.rho(); }

            void reset() { ndist_.reset(); }

            param_type param() const { return param_; }
            void param(const param_type & p) { param_ = p; }

            result_type min() const {
                return { -std::numeric_limits<FloatType>::infinity(),
                         -std::numeric_limits<FloatType>::infinity() };
            }
            result_type max() const {
                return { +std::numeric_limits<FloatType>::infinity(),
                         +std::numeric_limits<FloatType>::infinity() };
            }

            template<typename Engine>
            result_type operator()(Engine & engine) {
                return (*this)(engine, this->param_);
            }

            template<typename Engine>
            result_type operator()(Engine & engine, const param_type & p) {
                FloatType n1 = this->ndist_(engine);
                FloatType n2 = this->ndist_(engine);

                FloatType y1 = n1;
                FloatType y2 = p.r() * n1 + p.q() * n2;

                return {y1, y2};
            } /*operator()*/

            bool operator==(const gaussianpair_dist & x) const {
                return ((param_ == x.param_) && (ndist_ == x.ndist_));
            }
            bool operator!=(const gaussianpair_dist & x) const {
                return !(*this == x);
            }

        private:
            param_type param_;

            /* state for generating indept normally-distributed r.v's.
             * NB xo::rng::normal_dist, not std::normal_distribution: see
             * normal_dist.hpp
             */
            normal_dist<FloatType> ndist_;
        }; /*gaussianpair_dist*/

        /* generate pairs of correlated gaussian random variables */
        template<class Engine>
        class gaussianpairgen {
        public:
            using engine_type = Engine;
            using generator_type = generator<Engine, gaussianpair_dist<double>>;

            /* named ctor idiom */
            static generator_type make(Engine eng, double rho) {
                return generator_type::make(std::move(eng),
                                            gaussianpair_dist<double>(rho));
            }
        }; /*gaussianpairgen*/
    } /*namespace rng*/
} /*namespace xo*/

/* end gaussianpairgen.hpp */
