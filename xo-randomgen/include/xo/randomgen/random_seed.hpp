/* @file random_seed.hpp */

#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/pretty_struct.hpp>
#include <array>
#include <cstdint>
#include <stdlib.h>
#ifdef _BSD_SOURCE
# include <bsd/stdlib.h>
#else
# include <sys/random.h>
#endif

namespace xo {
    namespace rng {
        /* generate a 64-bit random seed using /dev/urandom or similar source.
         * This is relatively expensive;  at least cost of a system call
         * + may block if host has rebooted recently
         *
         * Require:
         * - T is null-constructible.
         *
         * return value will contain a T-instance in which representation
         * has been populated with random bits.   Expecting T to be something
         * like int32_t, or std::array<uint64_t, ..>
         */
        template<typename T>
        void random_seed(T * p_seed) {
#      ifdef __APPLE__
            /* NOTE: arc4random_buf() works on darwin/nix;
             *       probably need to do something else on intel linux
             */
            ::arc4random_buf(p_seed, sizeof(*p_seed));
#      else
            /* avail flags: GRND_RANDOM | GRND_NONBLOCK */
            while (::getrandom(p_seed, sizeof(*p_seed), 0) == -1) {
                if (errno == EINTR) {
                    /* interrupted by signal,  try again */
                    continue;
                } else {
                    break;
                }
            }
#      endif
        } /*random_seed*/

        template<typename T>
        T random_seed() {
            T retval;
            random_seed(&retval);

            return retval;
        } /*random_seed*/

        /** @brief RAII-style random-number seed
         *
         * Usage:
         * @code
         *   Seed<xoshiro256ss> seed;
         *
         *   auto eng = xoshiro256ss(seed);
         * @endcode
         *
         * or
         *
         * @code
         *   auto rng = UnitIntervalGen<xoshiro256ss>::make(seed);
         * @endcode
         */
        template <typename Engine>
        struct Seed {
            using PpSink = xo::pp::PpSink;

            using seed_type = typename Engine::seed_type;

            Seed() { random_seed(&seed_); }

            operator seed_type const & () const { return seed_; }

            void pretty(PpSink & ppsink) const;

            seed_type seed_;
        }; /*Seed*/

        template <typename Engine>
        void
        Seed<Engine>::pretty(PpSink & sink) const
        {
            sink.pretty_struct("seed", xo::pp::field("s", seed_));
        }

    } /*namespace rng*/
} /*namespace xo*/

namespace xo::pp {
    /* pretty() dispatches Prettifier<T> first and has no member detection,
     * so the member above is unreachable without this.
     */
    template <typename Engine>
    struct Prettifier<xo::rng::Seed<Engine>> {
        static void print(PpSink & sink, const xo::rng::Seed<Engine> & x) {
            x.pretty(sink);
        }
    };
} /*namespace xo::pp*/

/* end random_seed.hpp */
