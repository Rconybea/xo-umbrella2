/** @file DArena.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <string>

namespace xo {
    namespace mm {

        /** ArenaConfig
         **/
        struct ArenaConfig {
            /** optional name, for diagnostics **/
            std::string name_;
            /** arena size -- hard max = reserved virtual memory **/
            std::size_t size_;
            /** hugepage size -- using huge pages relieves some TLB pressure
             *  (provided you use their full extent :)
             **/
            std::size_t hugepage_z_ = 2 * 1024 * 1024;
            /** true to enable debug logging **/
            bool debug_flag_ = false;
        };

        /** Arena allocator state
         *
         *    <----------------------------size-------------------------->
         *    <------------committed-----------><-------uncommitted------>
         *    <--allocated-->
         *
         *    XXXXXXXXXXXXXXX___________________..........................
         *
         *    allocated:   in use
         *    committed:   physical memory obtained
         *    uncommitted: mapped in virtual memory, not backed by memory
         **/
        struct DArena {
            /** [lo, hi) already-mapped address range **/
            DArena(const ArenaConfig & cfg,
                   std::byte * lo,
                   std::byte * hi);

            ~DArena();

            ArenaConfig config_;

            /** size of a VM page (via getpagesize()). Likely 4k **/
            std::size_t page_z_ = 0;

            /** arena owns memory in range [@ref lo_, @ref hi_)
             **/
            std::byte * lo_ = nullptr;

            /** prefix of this size is committed.
             *  Remainder mapped but uncommitted.
             **/
            std::size_t committed_z_ = 0;

            /** free pointer.
             *  Memory in range [@ref lo_, @ref free_) current in use
             **/
            std::byte * free_ = nullptr;

            /** soft limit; end of committed virtual memory
             *  Memory in range [@ref lo_, @ref limit_) is committed
             *  (backed by physical memory)
             **/
            std::byte * limit_ = nullptr;

            /** hard limit; end of reserved virtual memory
             *  Memory in range [@ref limit_, @ref hi_) is uncommitted
             **/
            std::byte * hi_ = nullptr;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end DArena.hpp */
