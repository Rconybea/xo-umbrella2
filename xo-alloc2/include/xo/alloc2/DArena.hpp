/** @file DArena.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <string>

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
            /** true to store header (8 bytes) at the beginning of each allocation.
             *  necessary and sufficient to allows iterating over allocs
             *  present in arena
             **/
            bool store_header_flag_ = false;
            /** if non-zero, allocate extra space between allocs, and fill
             *  with fixed test-pattern contents. Allows for simple
             *  runtime arena sanitizing checks.
             *  Will be rounded up to multiple of @ref padding::c_alloc_alignment
             **/
            std::size_t guard_z_ = 0;
            /** if store_header_flag_ is true: mask bits for allocation size.
             *  remaining bits can be stolen for other purposes
             *  otherwise ignored
             **/
            std::uint64_t header_size_mask_ = 0xffffffff;
            /** true to enable debug logging **/
            bool debug_flag_ = false;

            ///@}
        };

        /** @class DArena
         *
         *  @brief represent arena allocator state
         *
         *  Provides minimal RAII functionality around memory mapping.
         *  For allocation implementation see @ref IAllocator_DArena
         **/
        struct DArena {
            /*
             *    <----------------------------size-------------------------->
             *    <------------committed-----------><-------uncommitted------>
             *    <--allocated-->
             *
             *    XXXXXXXXXXXXXXX___________________..........................
             *
             *    [X] allocated:   in use
             *    [_] committed:   physical memory obtained
             *    [.] uncommitted: mapped in virtual memory, not backed by memory
             */

            /** @defgroup mm-arena-traits arena type traits **/
            ///@{

            /** @brief an amount of memory **/
            using size_type = std::size_t;
            /** @brief a contiguous memory range **/
            using range_type = std::pair<std::byte*,std::byte*>;
            /** @brief type for allocation header (if enabled) **/
            using header_type = std::uint64_t;

            ///@}

            /** @defgroup mm-arena-ctors arena constructors and destructors **/
            ///@{

            /** create arena per configuration @p cfg. **/
            static DArena map(const ArenaConfig & cfg);

            /** ctor from already-mapped (but not committed) address range **/
            DArena(const ArenaConfig & cfg, size_type page_z, std::byte * lo, std::byte * hi);
            /** DArena is not copyable **/
            DArena(const DArena & other) = delete;
            /** move ctor **/
            DArena(DArena && other);
            /** dtor releases mapped memory **/
            ~DArena();

            ///@}

            /** obtain uncommitted contiguous memory range comprising
             *  a whole multiple of @p hugepage_z bytes, of at least size @p req_z,
             *  aligned on a @p hugepage_z boundary
             **/
            static range_type map_aligned_range(size_type req_z, size_type hugepage_z);

            /** @defgroup mm-arena-instance-vars **/
            ///@{

            /** arena configuration **/
            ArenaConfig config_;

            /** size of a VM page (obtained automatically via getpagesize()). Likely 4k **/
            size_type page_z_ = 0;

            /** arena owns memory in range [@ref lo_, @ref hi_)
             **/
            std::byte * lo_ = nullptr;

            /** prefix of this size is committed.
             *  Remainder mapped but uncommitted.
             **/
            size_type committed_z_ = 0;

            /** if config_.store_header_flag_:
             *  Pointer to header for last allocation.
             **/
            header_type * last_header_ = nullptr;

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

            /** count runtime errors. Each error updates @ref last_error_ **/
            uint32_t error_count_ = 0;

            /** capture some error details if/when error **/
            AllocatorError last_error_;

            ///@}
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end DArena.hpp */
