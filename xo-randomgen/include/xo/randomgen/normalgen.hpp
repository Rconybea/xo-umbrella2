/* @file normalgen.hpp */

#pragma once

#include "generator.hpp"
#include "normal_dist.hpp"

namespace xo {
    namespace rng {
        /* Engine: e.g. xo::rng::xoshiro256 or std::mt19937
         *
         * NB normal_dist, not std::normal_distribution: the latter's output
         * is not specified by the standard, and differs between libstdc++
         * and libc++.  See normal_dist.hpp.
         */
        template <class Engine>
        using normalgen = generator<Engine, normal_dist<double>>;
    } /*namespace rng*/
} /*namespace xo*/
