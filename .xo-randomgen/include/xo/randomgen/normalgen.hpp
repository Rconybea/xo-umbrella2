/* @file normalgen.hpp */

#pragma once

#include "generator.hpp"
#include <random>

namespace xo {
    namespace rng {
        /* Engine: e.g. xo::rng::xoshiro256 or std::mt19937 */
        template <class Engine>
        using normalgen = generator<Engine, std::normal_distribution<double>>;
    } /*namespace rng*/
} /*namespace xo*/
