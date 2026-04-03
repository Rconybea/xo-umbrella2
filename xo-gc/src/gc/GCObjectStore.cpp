/** @file GCObjectStore.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GCObjectStore.hpp"
#include <xo/indentlog/scope.hpp>
#include <cassert>

namespace xo {
    namespace mm {

        GCObjectStore::GCObjectStore(const GCObjectStoreConfig & cfg)
            : config_{cfg}
        {
            assert(config_.arena_config_.header_.size_bits_ +
                   config_.arena_config_.header_.age_bits_ +
                   config_.arena_config_.header_.tseq_bits_ <= 64);

            this->_init_space();
        }

        void
        GCObjectStore::_init_space()
        {
            assert(c_n_role == 2);

            for (uint32_t igen = 0, ngen = config_.n_generation_; igen < ngen; ++igen) {
                if (igen < c_max_generation) {
                    {
                        char buf[40];
                        snprintf(buf, sizeof(buf), "x1-space-G%u-a", igen);

                        this->space_storage_[0][igen]
                            = DArena::map(config_.arena_config_.with_name(std::string(buf)));
                    }
                    {
                        char buf[40];
                        snprintf(buf, sizeof(buf), "x1-space-G%u-b", igen);

                        this->space_storage_[1][igen]
                            = DArena::map(config_.arena_config_.with_name(std::string(buf)));
                    }

                    this->space_[role::to_space()][igen] = &space_storage_[0][igen];
                    this->space_[role::from_space()][igen] = &space_storage_[1][igen];
                } else {
                    assert(false);
                }
            }

            for (uint32_t igen = config_.n_generation_; igen < c_max_generation; ++igen) {
                this->space_[role::to_space()][igen] = nullptr;
                this->space_[role::from_space()][igen] = nullptr;
            }

            if (config_.n_generation_ == 2) {
                assert(this->get_space(role::to_space(), Generation{2}) == nullptr);
            }
        }

        Generation
        GCObjectStore::generation_of(role r, const void * addr) const noexcept
        {
            for (Generation gi{0}; gi < config_.n_generation_; ++gi) {
                const DArena * arena = this->get_space(r, gi);

                if (arena->contains(addr))
                    return gi;
            }

            return Generation::sentinel();
        }

        auto
        GCObjectStore::header2size(header_type hdr) const noexcept -> size_type
        {
            uint32_t z = config_.arena_config_.header_.size(hdr);

            return z;
        }

        object_age
        GCObjectStore::header2age(header_type hdr) const noexcept
        {
            uint32_t age = config_.arena_config_.header_.age(hdr);

            assert(age < c_max_object_age);

            return object_age(age);
        }

        uint32_t
        GCObjectStore::header2tseq(header_type hdr) const noexcept
        {
            uint32_t tseq = config_.arena_config_.header_.tseq(hdr);

            return tseq;
        }

        bool
        GCObjectStore::is_forwarding_header(header_type hdr) const noexcept
        {
            /** forwarding pointer encoded as sentinel tseq **/
            return config_.arena_config_.header_.is_forwarding_tseq(hdr);
        }

        void
        GCObjectStore::visit_pools(const MemorySizeVisitor & visitor) const
        {
            for (uint32_t j = 0; j < config_.n_generation_; ++j) {
                for (uint32_t i = 0; i < c_n_role; ++i) {
                    space_storage_[i][j].visit_pools(visitor);
                }
            }
        }

        void
        GCObjectStore::swap_roles(Generation upto) noexcept
        {
            scope log(XO_DEBUG(config_.debug_flag_), xtag("upto", upto));

            for (Generation g = Generation{0}; g < upto; ++g) {
                log && log("swap roles", xtag("g", g));

                std::swap(space_[role::to_space()][g], space_[role::from_space()][g]);
            }
        }

        void
        GCObjectStore::cleanup_phase(Generation upto,
                                     bool sanitize_flag)
        {
            scope log(XO_DEBUG(config_.debug_flag_), xtag("upto", upto));

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
