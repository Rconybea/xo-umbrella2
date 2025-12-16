/** @file AllocHeader.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace xo {
    namespace mm {
        struct AllocHeader {
            using repr_type = std::uintptr_t;
            using size_type = std::size_t;

            explicit AllocHeader(repr_type x) : repr_{x} {}

#ifdef OBSOLETE
            std::uint32_t tseq(const AllocHeaderConfig & cfg) const noexcept {
                return cfg.tseq(repr_);
            }

            std::uint32_t age(const AllocHeaderConfig & cfg) const noexcept {
                return cfg.age(repr_);
            }

            size_type size(const AllocHeaderConfig & cfg) const noexcept {
                return cfg.size(repr_);
            }
#endif

            repr_type repr_;
        };

        static_assert(sizeof(AllocHeader) == sizeof(AllocHeader::repr_type));
        static_assert(std::is_standard_layout_v<AllocHeader>);

        /*
         * Each allocation is preceded by a 64-bit header.
         * Header is split into 3 configurable-width bit fields,
         * labelled (from hi to lo bit order) {tseq, age, size}.
         *
         * 1. tseq. seq# identifying object types; needed for gc.
         * 2. gen.  age cohort; increases when alloc survives gc.
         * 3. size. alloc size.
         *
         * Arena allocator only uses size.
         * X1 collector uses {tseq, gen, size}
         *
         * alloc header
         *
         *  TTTTTTTTTTTTGGGGGZZZZZZZZZZZZ
         *  <   tseq   ><gen><   size   >
         *
         * masking
         *
         *  ..432107654321076543210 bit
         *
         *                 >       <  .gen_bits
         *  0..............01111111   gen_mask_unshifted
         *  0..011111110..........0   gen_mask_shifted
         *             >           <  gen_shift
         */
        struct AllocHeaderConfig {
            using repr_type = AllocHeader;

            AllocHeaderConfig() = default;
            AllocHeaderConfig(std::uint8_t t, std::uint8_t a, std::uint8_t z) noexcept
                : tseq_bits_{t}, age_bits_{a}, size_bits_{z} {}

            std::uint64_t tseq_mask() const noexcept {
                // e.g.
                //   FF FF FF 00 00 00 00 00
                // with tseq_bits=24, age_bits=8, size_bits=32
                //
                return ((1ul << tseq_bits_) - 1) << (age_bits_ + size_bits_);
            }

            std::uint64_t age_mask() const noexcept {
                // e.g.
                //   00 00 00 FF 00 00 00 00
                // with age_bits=8, size_bits=32
                //
                return ((1ul << age_bits_) - 1) << size_bits_;
            }

            std::uint64_t size_mask() const noexcept {
                // e.g.
                //   00 00 00 00 FF FF FF FF
                // with size_bits=32
                //
                return ((1ul << size_bits_) - 1);
            }

            /** extract type id from alloc header @p hdr **/
            std::uint32_t tseq(repr_type hdr) const noexcept {
                // e.g.
                //   0x302010
                // for header
                //   30 20 10 -- -- -- -- --
                // with tseq_bits_ = 24, age_bits_ + size_bits_ = 40
                //
                return (hdr.repr_ & tseq_mask()) >> (age_bits_ + size_bits_);
            }

            /** extract age from alloc header @p hdr **/
            std::uint32_t age(repr_type hdr) const noexcept {
                // e.g.
                //  0xa0
                // for header
                //   -- -- -- a0 -- -- -- --
                // with age_bits_ = 8, size_bits_ = 32
                //
                return (hdr.repr_ & age_mask()) >> size_bits_;
            }

            /** extract size from alloc header @p hdr **/
            std::size_t size(repr_type hdr) const noexcept {
                // e.g.
                //  0x01020300
                // for header
                //   -- -- -- -- 01 02 03 00
                // with size_bits_ = 32
                //
                return (hdr.repr_ & size_mask());
            }

            /** true iff sentinel tseq, flagging a forwarding pointer **/
            bool is_forwarding_tseq(repr_type hdr) const noexcept {
                // e.g.
                //   0xFFFFFF
                // i.e. header
                //   FF FF FF -- -- -- -- --
                // with tseq_bits_ = 24, age_bits + size_bits_ = 40
                //
                return (hdr.repr_ & tseq_mask()) == tseq_mask();
            }

            bool is_size_enabled() const noexcept { return size_bits_ > 0; }

            /** number of bits for tseq **/
            std::uint8_t tseq_bits_ = 24;
            /** number of bits for age **/
            std::uint8_t age_bits_ = 8;
            /** number of bits for size **/
            std::uint8_t size_bits_ = 32;
        };

    }
}

/* end AllocHeader.hpp */
