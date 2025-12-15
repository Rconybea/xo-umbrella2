/** @file ArenaConfig.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "alloc/AllocatorError.hpp"
#include <string>
#include <cstdint>

namespace xo {
    namespace mm {

        /** @class ArenaConfig
         *
         *  @brief configuration for a @ref DArena instance
         **/
        struct ArenaConfig {
            /** @defgroup mm-arenaconfig-instance-vars ArenaConfig members **/
            ///@{

            /** optional name, for diagnostics **/
            std::string name_;
            /** desired arena size -- hard max = reserved virtual memory **/
            std::size_t size_;
            /** hugepage size -- using huge pages relieves some TLB pressure
             *  (provided you use their full extent :)
             **/
            std::size_t hugepage_z_ = 2 * 1024 * 1024;
            /** if non-zero, allocate extra space between allocs, and fill
             *  with fixed test-pattern contents. Allows for simple
             *  runtime arena sanitizing checks.
             *  Will be rounded up to multiple of @ref padding::c_alloc_alignment
             **/
            std::size_t guard_z_ = 0;
            /** if guard_z_ > 0, write at least that many copies
             *  of this guard byte following each complete allocation
             **/
            std::uint8_t guard_byte_ = 0xfd;
            /** if store_header_flag_ is true: mask bits for allocation size.
             *  remaining bits can be stolen for other purposes
             *  otherwise ignored
             **/
            /** true to store header (8 bytes) at the beginning of each allocation.
             *  necessary and sufficient to allows iterating over allocs
             *  present in arena
             **/
            bool store_header_flag_ = false;
            /** number of bits to represent allocation size **/
            std::uint64_t header_size_bits_ = 32;
            std::uint64_t header_size_mask_ = (1ul << header_size_bits_) - 1;
            /** true to enable debug logging **/
            bool debug_flag_ = false;

            ///@}
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end ArenaConfig.hpp */
