/** @file ArenaConfig.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AllocHeaderConfig.hpp"
#include "MemoryNameStr.hpp"
#include <xo/flatstring/flatstring.hpp>
#include <xo/ppsink/Prettifier.hpp>
#include <cstdint>
#include <string>

namespace xo {
    namespace pp { class PpSink; }

    namespace mm {

        class DArena;
        using ArenaNameStr = MemoryNameStr;

        /** @class ArenaConfig
         *
         *  @brief configuration for a @ref DArena instance
         **/
        struct ArenaConfig {
            using PpSink = xo::pp::PpSink;
            using size_type = std::size_t;

            /** @defgroup mm-arenaconfig-ctors **/
            ///@{

            /** NOTE: not providing explicit ctors so we can use designated initializers **/

            ArenaConfig with_name(const ArenaNameStr & name) const;
            ArenaConfig with_size(std::size_t z) const;
            ArenaConfig with_base_align_z(std::size_t z) const;
            ArenaConfig with_exclusive_block_flag(bool x) const;
            ArenaConfig with_store_header_flag(bool x) const;

            ///@}
            /** @defgroup mm-arenaconfig-access-methods ArenaConfig access methods **/
            ///@{

            const MemoryNameStr & name() const { return name_; }
            size_type preamble_z() const { return sizeof(DArena*) + header_.guard_z_; }
            size_type alloc_header_z() const { return store_header_flag_ ? sizeof(AllocHeader) : 0; }
            size_type guard_z() const { return header_.guard_z_; }
            size_type padded_guard_z() const { return padding::with_padding(header_.guard_z_); }
            /** hard max arena size **/
            size_type size() const { return size_; }

            ///@}
            /** @defgroup mm-arenaconfig-methods ArenaConfig methods **/
            ///@{

            /** pretty print instance to @p sink **/
            void pretty(PpSink & sink) const;

            ///@}
            /** @defgroup mm-arenaconfig-instance-vars ArenaConfig members **/
            ///@{

        public:
            /** optional name, for diagnostics **/
            MemoryNameStr name_;
            /** desired arena size -- hard max = reserved virtual memory **/
            std::size_t size_ = 0;
            /** desired alignment for this arena's BASE address.
             *  0 for the page/hugepage default.
             *
             *  Allow recovering an arena's base from any pointer in that arena,
             *  by masking off the low-order @c base_align_z_-1
             *  bits. This is sound only if @code size_ <= base_align_z_ @endcode
             *  See @ref with_base_align_z.
             **/
            std::size_t base_align_z_ = 0;
            /** when @ref base_align_z_ > 0:
             *  true to reserve the whole @ref base_align_z_ block rather than
             *  just @ref size_. Applies to address space, not committed memory.
             *  Ignored when @ref base_align_z_ is zero.
             *
             *  See @ref with_exclusive_block_flag.
             **/
            bool exclusive_block_flag_ = false;
            /** hugepage size -- using huge pages relieves some TLB pressure
             *  (provided you use their full extent :)
             **/
            std::size_t hugepage_z_ = 2 * 1024 * 1024;
            /** true to store header (8 bytes) at the beginning of each allocation.
             *  necessary and sufficient to allows iterating over allocs
             *  present in arena.
             **/
            bool store_header_flag_ = false;
            /** configuration for per-alloc header **/
            AllocHeaderConfig header_{};
            /** true to enable debug logging **/
            bool debug_flag_ = false;

            ///@}
        };

    } /*namespace mm*/

    namespace pp {
        /** pretty-print for ArenaConfig **/
        template <>
        struct Prettifier<xo::mm::ArenaConfig> {
            static void print(PpSink & sink, const xo::mm::ArenaConfig & x) {
                x.pretty(sink);
            }
        };
    } /*namespace pp*/
} /*namespace xo*/

/* end ArenaConfig.hpp */
