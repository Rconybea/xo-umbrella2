/* @file ex2.cpp */

#include "xo/randomgen/xoshiro256.hpp"
#include "xo/randomgen/random_seed.hpp"

using namespace xo;
using namespace xo::rng;

int
main(int argc, char ** argv) {
    Seed<xoshiro256ss> seed;

    xoshiro256ss eng(seed);
} /*main*/

/* end ex2.cpp */
