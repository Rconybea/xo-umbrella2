/* @file ex1.cpp */

#include "xo/randomgen/xoshiro256.hpp"
#include <algorithm>
#include <iostream>

using namespace xo;
using namespace xo::rng;

int
main(int argc, char ** argv) {
    xoshiro256ss rng{123456789};

    std::array<std::uint64_t, 20> v;

    std::generate(v.begin(), v.end(), rng);

    for (std::uint64_t i=0; i<v.size(); ++i)
        std::cout << "v[" << i << "]: " << v[i] << std::endl;

    return 0;
} /*main*/

/* end ex1.cpp */
