/** @file GCObjectStore.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GCObjectStore.hpp"
#include <xo/indentlog/scope.hpp>
#include <cassert>

namespace xo {
    namespace mm {

        GCObjectStore::GCObjectStore(const ArenaConfig & arena_cfg,
                                     uint32_t ngen, bool debug_flag)
        : arena_config_{arena_cfg},
          n_generation_{ngen},
          debug_flag_{debug_flag}
        {
            assert(arena_config_.header_.size_bits_ +
                   arena_config_.header_.age_bits_ +
                   arena_config_.header_.tseq_bits_ <= 64);

            this->_init_space();
        }

        void
        GCObjectStore::_init_space()
        {
            assert(c_n_role == 2);

            for (uint32_t igen = 0, ngen = n_generation_; igen < ngen; ++igen) {
                if (igen < c_max_generation) {
                    {
                        char buf[40];
                        snprintf(buf, sizeof(buf), "x1-space-G%u-a", igen);

                        this->space_storage_[0][igen]
                            = DArena::map(arena_config_.with_name(std::string(buf)));
                    }
                    {
                        char buf[40];
                        snprintf(buf, sizeof(buf), "x1-space-G%u-b", igen);

                        this->space_storage_[1][igen]
                            = DArena::map(arena_config_.with_name(std::string(buf)));
                    }

                    this->space_[role::to_space()][igen] = &space_storage_[0][igen];
                    this->space_[role::from_space()][igen] = &space_storage_[1][igen];
                } else {
                    assert(false);
                }
            }

            for (uint32_t igen = n_generation_; igen < c_max_generation; ++igen) {
                this->space_[role::to_space()][igen] = nullptr;
                this->space_[role::from_space()][igen] = nullptr;
            }

            if (n_generation_ == 2) {
                assert(this->get_space(role::to_space(), Generation{2}) == nullptr);
            }
        }

        void
        GCObjectStore::visit_pools(const MemorySizeVisitor & visitor) const
        {
            for (uint32_t j = 0; j < n_generation_; ++j) {
                for (uint32_t i = 0; i < c_n_role; ++i) {
                    space_storage_[i][j].visit_pools(visitor);
                }
            }
        }

        void
        GCObjectStore::swap_roles(Generation upto) noexcept
        {
            scope log(XO_DEBUG(true), xtag("upto", upto));

            for (Generation g = Generation{0}; g < upto; ++g) {
                log && log("swap roles", xtag("g", g));

                std::swap(space_[role::to_space()][g], space_[role::from_space()][g]);
            }
        }

        void
        GCObjectStore::cleanup_phase(Generation upto,
                                     bool sanitize_flag)
        {
            scope log(XO_DEBUG(true), xtag("upto", upto));

            // everything live has been copied out of from-space
            // -> now set to empty
            //
            for (Generation g = Generation{0}; g < upto; ++g) {
                if (sanitize_flag) {
                    space_[role::from_space()][g]->scrub();
                }

                space_[role::from_space()][g]->clear();
            }
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end GCObjectStore.cpp */
