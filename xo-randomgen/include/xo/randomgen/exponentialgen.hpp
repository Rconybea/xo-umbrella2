/* @file exponentialgen.hpp */

#pragma once

#include "generator.hpp"
#include <random>

namespace xo {
    namespace rng {
        template <class Engine>
        class exponentialgen : public generator<Engine, std::exponential_distribution<double>> {
        public:
            using generator_type = generator<Engine, std::exponential_distribution<double>>;

            template <class Engine>
            static generator_type make(Engine eng, double lambda) {
                return make_generator(std::move(eng), std::exponential_distribution<double>(lambda));
            }
        };
    } /*namespace rng*/
} /*namespace xo*/
