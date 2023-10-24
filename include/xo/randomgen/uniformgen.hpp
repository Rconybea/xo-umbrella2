/* @file uniformgen.hpp */

#pragma once

#include "generator.hpp"
#include <random>

namespace xo {
    namespace rng {
        template <class Engine>
        class uniformgen : public generator<Engine, std::uniform_real_distribution<double>> {
        public:
            using generator_type = generator<Engine, std::uniform_real_distribution<double>>;

            template <class Engine>
            static generator_type unit(Engine eng) {
                return make_generator(std::move(eng),
                                      std::uniform_real_distribution<double>(0.0, 1.0));
            }
        };
    } /*namespace rng*/
} /*namespace xo*/


/* end uniformgen.hpp */
