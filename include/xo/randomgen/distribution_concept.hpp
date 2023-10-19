/* @file distribution_concept.hpp */

#pragma once

#include <concepts>

namespace xo {
    namespace rng {
        template <class Distribution, class Engine>
        concept distribution_concept = requires(Distribution dist, typename Distribution::param_type p) {
            typename Distribution::result_type;
            typename Distribution::param_type;
            { Distribution() };
            { Distribution(p) };
            { dist.reset() };
            { dist.param() };
            { dist.param(p) };
            // { dist(g) };   // generator g satisfying engine_concept
            // { dist(g, p) };
            { dist.min() };
            { dist.max() };
            { dist == dist };
            { dist != dist };
            // os << dist
            // is >> dist

        }
#ifdef __clang__
        // std::copyable apparently not available in clang11 ?
#else
        && std::copyable<Distribution>
        && std::copyable<typename Distribution::param_type>
        && std::equality_comparable<typename Distribution::param_type>
#endif
        ;
    } /*namespace rng*/

} /*namespace xo*/

/* end distribution_concept.hpp */
