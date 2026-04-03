/** @file GCObjectStore.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GCObjectStore.hpp"

namespace xo {
    namespace mm {

        GCObjectStoreConfig::GCObjectStoreConfig(const ArenaConfig & arena_cfg,
                                                 std::uint32_t ngen,
                                                 bool debug_flag)
            : arena_config_{arena_cfg},
              n_generation_{ngen},
              debug_flag_{debug_flag}
        {}

    } /*namespace mm*/
} /*namespace xo*/

/* end GCObjectStore.cpp */
