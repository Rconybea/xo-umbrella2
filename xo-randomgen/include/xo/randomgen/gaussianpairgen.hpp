/* @file gaussianpairgen.hpp */

#pragma once

#include "generator.hpp"
#include <random>
#include <array>

namespace xo {
    namespace random {
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

        public:
            /* generate pairs of gaussian N(0,1) random numbers,
             * with correlation coefficient rho
             *
             * Require:
             * - rho in the interval [-1, +1]
             */
            explicit gaussianpair_dist(FloatType rho)
                : r_(rho), q_(std::sqrt(1.0 - rho*rho)) {}

            template<typename Engine>
            result_type operator()(Engine & engine) {
                FloatType n1 = this->ndist_(engine);
                FloatType n2 = this->ndist_(engine);

                FloatType y1 = n1;
                FloatType y2 = this->r_ * n1 + this->q_ * n2;

                return {y1, y2};
            } /*operator()*/

        private:
            /* correlation coefficient r
             * 2nd random variable Y2 in each pair will be constructed by
             * r.N1 + sqrt(1-r^2).N2
             */
            FloatType r_;
            /* q := sqrt(1-r^2) */
            FloatType q_;

            /* state for generating indept normally-distributed r.v's */
            std::normal_distribution<FloatType> ndist_;
        }; /*gaussianpair_dist*/

        /* generate pairs of correlated gaussian random variables */
        template<class Engine>
        class gaussianpairgen {
        public:
            using engine_type = Engine;
            using generator_type = generator<Engine, gaussianpair_dist<double>>;

            template<typename Engine>
            static generator_type make(Engine eng,
                                       double rho) {
                return generator_type::make(std::move(eng),
                                            gaussianpair_dist<double>(rho));
            }
        }; /*GaussianPairGen*/
    } /*namespace random*/
} /*namespace xo*/

/* end gaussianpairgen.hpp */
