/* @file generator.hpp */

#pragma once

#include "engine_concept.hpp"
#include "distribution_concept.hpp"
#include <utility>

namespace xo {
    namespace rng {
        /* Engine:        uniform integer random number generator,  e.g. xoshiro256ss
         * Distribution:  random number distribution,  e.g. std::normal_distribution
         */
        template <class Engine, class Distribution> requires engine_concept<Engine> && distribution_concept<Distribution, Engine>
        class generator {
        public:
            using result_type = typename Distribution::result_type;
            using engine_type = Engine;

        public:
            generator(Engine & e, Distribution const & d)
                : engine_{e},
                  distribution_{d} {}
            generator(Engine && e, Distribution && d)
                : engine_{std::move(e)},
                  distribution_{std::move(d)} {}

            static generator make(Engine e, Distribution d) {
                return generator(e, d);
            }

            result_type operator()() { return this->distribution_(this->engine_); }

        private:
            /* random number generator;  generates uniformly-distributed integers */
            Engine engine_;
            /* distribution object */
            Distribution distribution_;
        }; /*generator*/

        template <class Engine, class Distribution>
        generator<Engine, Distribution> make_generator(Engine e, Distribution d) {
            return generator<Engine, Distribution>::make(std::move(e),
                                                         std::move(d));
        } /*make_generator*/
    } /*namespace rng*/
} /*namespace xo*/

/* end generator.hpp */
