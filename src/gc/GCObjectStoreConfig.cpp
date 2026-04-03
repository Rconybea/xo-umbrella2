/** @file GCObjectStore.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GCObjectStore.hpp"

namespace xo {
    namespace mm {

        GCObjectStoreConfig::GCObjectStoreConfig(const ArenaConfig & arena_cfg,
                                                 std::uint32_t ngen,
                                                 std::uint32_t nsurvive,
                                                 bool debug_flag)
            : arena_config_{arena_cfg},
              n_generation_{ngen},
              n_survive_threshold_{nsurvive},
              debug_flag_{debug_flag}
        {}

    } /*namespace mm*/
} /*namespace xo*/

/* end GCObjectStore.cpp */
