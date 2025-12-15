/** @file DX1Collector.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "gc/DX1Collector.hpp"
#include "gc/generation.hpp"
#include <_types/_uint32_t.h>
#include <cassert>
#include <cstdint>

namespace xo {
    namespace mm {
#ifdef NOT_USING
        constexpr std::uint64_t
        CollectorConfig::gen_mult() const {
            return 1ul << arena_config_.header_size_bits_;
        }
#endif

        constexpr std::uint64_t
        CollectorConfig::gen_shift() const {
            return arena_config_.header_size_bits_;
        }

        constexpr std::uint64_t
        CollectorConfig::gen_mask_unshifted() const {
            return (1ul << gen_bits_) - 1;
        }

        constexpr std::uint64_t
        CollectorConfig::gen_mask_shifted() const {
            return gen_mask_unshifted() << arena_config_.header_size_bits_;
        }

#ifdef NOT_USING
        constexpr std::uint64_t
        CollectorConfig::tseq_mult() const {
            return 1ul << (gen_bits_ + arena_config_.header_size_bits_);
        }
#endif

        constexpr std::uint64_t
        CollectorConfig::tseq_shift() const {
            return gen_bits_ + arena_config_.header_size_bits_;
        }

        constexpr std::uint64_t
        CollectorConfig::tseq_mask_unshifted() const {
            return (1ul << tseq_bits_) - 1;
        }

        constexpr std::uint64_t
        CollectorConfig::tseq_mask_shifted() const {
            return tseq_mask_unshifted() << (gen_bits_ + arena_config_.header_size_bits_);
        }

        // ----- GCRunState -----

        GCRunState::GCRunState(generation gc_upto)
            : gc_upto_{gc_upto}
        {}

        GCRunState
        GCRunState::gc_not_running()
        {
            return GCRunState(generation(0));
        }

        GCRunState
        GCRunState::gc_upto(generation g)
        {
            return GCRunState(generation(g + 1));
        }

        // ----- DX1Collector -----

        DX1Collector::DX1Collector(const CollectorConfig & cfg) : config_{cfg}
        {
            assert(config_.arena_config_.header_size_bits_ + config_.gen_bits_ + config_.tseq_bits_ <= 64);

            for (uint32_t igen = 0, ngen = cfg.n_generation_; igen < ngen; ++igen) {
                space_storage_[0][igen] = DArena::map(cfg.arena_config_);
                space_storage_[1][igen] = DArena::map(cfg.arena_config_);

                space_[role::to_space()][igen] = &space_storage_[0][igen];
                space_[role::from_space()][igen] = &space_storage_[1][igen];
            }

            for (uint32_t igen = cfg.n_generation_; igen < c_max_generation; ++igen) {
                space_[role::to_space()][igen] = nullptr;
                space_[role::from_space()][igen] = nullptr;
            }
        }

        bool
        DX1Collector::contains(role r, void * addr) const
        {
            for (generation gi{0}; gi < config_.n_generation_; ++gi) {
                if (get_space(r, gi)->contains(addr))
                    return true;
            }

            return false;
        }

        std::size_t
        DX1Collector::header2size(header_type hdr) const
        {
            uint32_t z = (hdr & config_.arena_config_.header_size_mask_);

            return z;
        }

        generation
        DX1Collector::header2gen(header_type hdr) const
        {
            uint32_t g = (hdr & config_.gen_mask_shifted()) >> config_.gen_shift();

            assert(g < c_max_generation);

            return generation(g);
        }

        uint32_t
        DX1Collector::header2tseq(header_type hdr) const
        {
            uint32_t tseq = (hdr & config_.tseq_mask_shifted()) >> config_.tseq_shift();

            return tseq;
        }

        bool
        DX1Collector::is_forwarding_header(header_type hdr) const
        {
            /** all 1 bits to flag forwarding pointer **/
            return header2tseq(hdr) == config_.tseq_mask_shifted();
        }

        void
        DX1Collector::reverse_roles(generation g) {
            assert(g < config_.n_generation_);

            std::swap(space_[0][g], space_[1][g]);
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end DX1Collector.cpp */
