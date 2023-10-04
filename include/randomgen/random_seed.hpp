/* @file random_seed.hpp */

//#include "indentlog/print/array.hpp"
#include <iostream>
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
#      ifdef _BSD_SOURCE
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

        /* RAII-style random-number seed
         *
         * Usage:
         *
         *   Seed<xoshiro256ss> seed;
         *
         *   auto eng = xoshiro256ss(seed);
         * or
         *   auto rng = UnitIntervalGen<xoshiro256ss>::make(seed);
         */
        template<typename Engine>
        struct Seed {
            using seed_type = typename Engine::seed_type;

            Seed() { random_seed(&seed_); }

            operator seed_type const & () const { return seed_; }

            seed_type seed_;
        }; /*Seed*/

        template<typename T>
        inline std::ostream &
        operator<<(std::ostream & os,
                   Seed<T> const & x)
        {
            os << x.seed_;
            return os;
        } /*operator<<*/

    } /*namespace rng*/
} /*namespace xo*/

/* end random_seed.hpp */
