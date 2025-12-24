/** @file ArenaConfig.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "ArenaConfig.hpp"
namespace xo {
    namespace mm {
        ArenaConfig
        ArenaConfig::simple(std::size_t z)
        {
            return ArenaConfig { .size_ = z };
        }
    }
}

/* end ArenaConfig.cpp */
