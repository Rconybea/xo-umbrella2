/** @file DX1Collector.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "Allocator.hpp"
#include "arena/IAllocator_DArena.hpp"
#include "xo/gc/DX1Collector.hpp"
#include "xo/gc/DX1CollectorIterator.hpp"
#include "generation.hpp"
#include "object_age.hpp"
#include <xo/facet/obj.hpp>
#include <xo/indentlog/scope.hpp>
#include <cassert>
#include <cstdint>

namespace xo {
    using xo::mm::AAllocator;
    using xo::facet::with_facet;

    namespace mm {
#ifdef NOT_USING
        constexpr std::uint64_t
        CollectorConfig::gen_mult() const {
            return 1ul << arena_config_.header_size_bits_;
        }
#endif

#ifdef NOT_USING
        constexpr std::uint64_t
        CollectorConfig::tseq_mult() const {
            return 1ul << (gen_bits_ + arena_config_.header_size_bits_);
        }
#endif

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

        using size_type = xo::mm::DX1Collector::size_type;

        DX1Collector::DX1Collector(const CollectorConfig & cfg) : config_{cfg}
        {
            assert(config_.arena_config_.header_.size_bits_ +
                   config_.arena_config_.header_.age_bits_ +
                   config_.arena_config_.header_.tseq_bits_ <= 64);

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
        DX1Collector::contains(role r, const void * addr) const noexcept
        {
            for (generation gi{0}; gi < config_.n_generation_; ++gi) {
                const DArena * arena = get_space(r, gi);

                if (arena->contains(addr))
                    return true;
            }

            return false;
        }

        AllocError
        DX1Collector::last_error() const noexcept
        {
            // TODO:
            // need to adjust here if runtime errors
            // encountered during gc.

            return get_space(role::to_space(), generation::nursery())->last_error_;
        }

        namespace {
            size_type
            accumulate_total_aux(const DX1Collector & d,
                                 size_t (DArena::* get_stat_fn)() const) noexcept
            {
                size_t z = 0;

                for (role ri : role::all()) {
                    for (generation gj{0}; gj < d.config_.n_generation_; ++gj) {
                        const DArena * arena = d.get_space(ri, gj);

                        assert(arena);

                        z += (arena->*get_stat_fn)();
                    }
                }

                return z;
            }
        }

        size_type
        DX1Collector::reserved_total() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::reserved);
        }

        size_type
        DX1Collector::size_total() const noexcept
        {
            return committed_total();
        }

        size_type
        DX1Collector::committed_total() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::committed);
        }

        size_type
        DX1Collector::available_total() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::available);
        }

        size_type
        DX1Collector::allocated_total() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::allocated);
        }

        size_type
        DX1Collector::header2size(header_type hdr) const noexcept
        {
            uint32_t z = config_.arena_config_.header_.size(hdr);

            return z;
        }

        object_age
        DX1Collector::header2age(header_type hdr) const noexcept
        {
            uint32_t age = config_.arena_config_.header_.age(hdr);

            assert(age < c_max_object_age);

            return object_age(age);
        }

        uint32_t
        DX1Collector::header2tseq(header_type hdr) const noexcept
        {
            uint32_t tseq = config_.arena_config_.header_.tseq(hdr);

            return tseq;
        }

        bool
        DX1Collector::is_forwarding_header(header_type hdr) const noexcept
        {
            /** forwarding pointer encoded as sentinel tseq **/
            return config_.arena_config_.header_.is_forwarding_tseq(hdr);
        }

        auto
        DX1Collector::alloc(size_type z) noexcept -> value_type
        {
            return with_facet<AAllocator>::mkobj(new_space()).alloc(z);
        }

        auto
        DX1Collector::super_alloc(size_type z) noexcept -> value_type {
            return with_facet<AAllocator>::mkobj(new_space()).super_alloc(z);
        }

        auto
        DX1Collector::sub_alloc(size_type z, bool complete) noexcept -> value_type {
            return with_facet<AAllocator>::mkobj(new_space()).sub_alloc(z, complete);
        }

        bool
        DX1Collector::expand(size_type z) noexcept
        {
            if (with_facet<AAllocator>::mkobj(to_space(generation{0})).expand(z))
                return with_facet<AAllocator>::mkobj(from_space(generation{0})).expand(z);

            return false;
        }

        AllocInfo
        DX1Collector::alloc_info(value_type mem) const noexcept {
            for (role ri : role::all()) {
                for (generation gj{0}; gj < config_.n_generation_; ++gj) {
                    const DArena * arena = this->get_space(ri, gj);

                    assert(arena);

                    if (arena->contains(mem)) {
                        return arena->alloc_info(mem);
                    }
                }
            }

            // deliberately attempt on nursery to-space, to capture error info + return sentinel
            return this->get_space(role::to_space(), generation{0})->alloc_info(mem);
        }

        DX1CollectorIterator
        DX1Collector::begin() const noexcept
        {
            scope log(XO_DEBUG(false));

            const DArena * arena
                = get_space(role::to_space(),
                            generation{0});

            return DX1CollectorIterator(this,
                                        generation{0},
                                        generation{config_.n_generation_},
                                        arena->begin(),
                                        arena->end());
        }

        DX1CollectorIterator
        DX1Collector::end() const noexcept {
            scope log(XO_DEBUG(false));

            generation gen_hi = generation{config_.n_generation_};

            /** valid iterator for end points to end of last DArena.
             *  otherwise will interfere with working compare
             *  (since invalid iterators are incomparable)
             **/

            const DArena * arena
                = get_space(role::to_space(),
                            generation(config_.n_generation_ - 1));
            DArenaIterator arena_end = arena->end();

            return DX1CollectorIterator(this,
                                        gen_hi,
                                        gen_hi,
                                        arena_end,
                                        arena_end);
        }

        void
        DX1Collector::reverse_roles(generation g) noexcept {
            assert(g < config_.n_generation_);

            std::swap(space_[0][g], space_[1][g]);
        }

        void
        DX1Collector::clear() noexcept {
            for (role ri : role::all()) {
                for (generation gj{0}; gj < config_.n_generation_; ++gj) {
                    DArena * arena = this->get_space(ri, gj);

                    assert(arena);

                    arena->clear();
                }
            }
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end DX1Collector.cpp */
