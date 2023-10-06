/* @file engine_concept.hpp */

#pragma once

#include <concepts>
#include <random>

namespace xo {
    namespace rng {
        /* an engine generates psuedo-random bits.
         * given
         *   RngEngine eng = ...;
         *
         *   RngEngine::result_type x = eng();
         *
         * puts random bits into x.
         */
        template <class RngEngine>
        concept engine_concept = requires(RngEngine engine, typename RngEngine::result_type r) {
            /* note: the first 4 requirements characterize UniformRandomBitGenerator */
            typename RngEngine::result_type;
            { RngEngine(r) };
            { engine.min() } -> std::same_as<typename RngEngine::result_type>;
            { engine.max() } -> std::same_as<typename RngEngine::result_type>;
            /* must return value in closed interval [.min(), .max()] */
            { engine() } -> std::same_as<typename RngEngine::result_type>;

            { engine.seed() };
            { engine.seed(r) };
            { engine == engine };
            { engine != engine };
        } && std::copyable<RngEngine> && std::uniform_random_bit_generator<RngEngine>;
    } /*namespace rng*/
} /*namespace xo*/

/* end engine_concept.hpp */
