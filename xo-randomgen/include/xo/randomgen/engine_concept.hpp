/* @file engine_concept.hpp */

#pragma once

#include <concepts>
#include <random>

namespace std {
#ifdef __clang__

# if __clang_major__ <= 11
    template < class T >
    concept integral = std::is_integral_v<T>;

    template < class T >
    concept signed_integral = std::integral<T> && std::is_signed_v<T>;

    template < class T >
    concept unsigned_integral
    = std::integral<T> && !std::signed_integral<T>;

    template< class F, class... Args >
    concept invocable
    = requires(F&& f, Args&&... args) {
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        /* not required to be equality-preserving */
    };

    template< typename G >
    concept uniform_random_bit_generator
    = std::invocable<G&>
        && std::unsigned_integral<std::invoke_result_t<G&>>
        && requires { { G::min() } -> std::same_as<std::invoke_result_t<G&>>;
                      { G::max() } -> std::same_as<std::invoke_result_t<G&>>;
                      requires std::bool_constant<(G::min() < G::max())>::value; };
# endif

#else
    /* uniform_random_bit_generator provided by gcc 12.3.2 */
    /* uniform_random_bit_generator provided by clang 16 */
#endif
} /*namespace std*/

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
        }
#ifdef __clang__
        // std::copyable apparently not available in clang11 ?
#else
            && std::copyable<RngEngine>
#endif
        && std::uniform_random_bit_generator<RngEngine>;
    } /*namespace rng*/
} /*namespace xo*/

/* end engine_concept.hpp */
