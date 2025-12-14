/** @file DX1Collector.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "gc/DX1Collector.hpp"
#include <cassert>

namespace xo {
    namespace mm {
        DX1Collector::DX1Collector(const CollectorConfig & cfg) : config_{cfg}
        {
            for (uint32_t igen = 0, ngen = cfg.n_generation_; igen < ngen; ++igen) {
                space_storage_[0][igen] = std::move(DArena::map(cfg.arena_config_));
                space_storage_[1][igen] = std::move(DArena::map(cfg.arena_config_));

                space_[role::to_space()][igen] = &space_storage_[0][igen];
                space_[role::from_space()][igen] = &space_storage_[1][igen];
            }

            for (uint32_t igen = cfg.n_generation_; igen < c_max_generation; ++igen) {
                space_[role::to_space()][igen] = nullptr;
                space_[role::from_space()][igen] = nullptr;
            }
        }

        void
        DX1Collector::reverse_roles(generation g) {
            assert(g < config_.n_generation_);

            std::swap(space_[0][g], space_[1][g]);
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end DX1Collector.cpp */
