/* @file bernoulligen.hpp */

#pragma once

#include "generator.hpp"
#include <random>

namespace xo {
    namespace rng {
        /* Engine: e.g. xo::rng::xoshiro256ss or std::mt19937 */
        template <class Engine>
        class bernoulligen : public generator<Engine, std::bernoulli_distribution<double>> {
        public:
            using generator_type = generator<Engine, std::bernoulli_distribution<double>>;

            template <class Engine>
            static generator_type make(Engine engine, double prob) {
                return generator_type::make(std::move(engine),
                                            std::bernoulli_distribution<double>(prob));
            }

            template <class Engine>
            static generator_type conflip(Engine engine) {
                return generator_type::make(std::move(engine),
                                            std::bernoulli_distribution<double>(0.5));
            }
        };
    } /*namespace rng*/
 } /*namespace xo*/
