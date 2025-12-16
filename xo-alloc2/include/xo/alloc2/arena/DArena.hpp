/** @file DArena.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "ArenaConfig.hpp"

namespace xo {
    namespace mm {

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
            /** @brief allocation pointer; use for allocation results  **/
            using value_type = std::byte*;
            /** @brief a contiguous memory range **/
            using range_type = std::pair<value_type, value_type>;
            /** @brief type for allocation header (if enabled) **/
            using header_type = AllocHeader; //std::uint64_t;

            ///@}

            /** @defgroup mm-arena-ctors arena constructors and destructors **/
            ///@{

            /** create arena per configuration @p cfg. **/
            static DArena map(const ArenaConfig & cfg);

            /** null ctor **/
            DArena() = default;
            /** ctor from already-mapped (but not committed) address range **/
            DArena(const ArenaConfig & cfg, size_type page_z, value_type lo, value_type hi);
            /** DArena is not copyable **/
            DArena(const DArena & other) = delete;
            /** move ctor **/
            DArena(DArena && other);
            /** dtor releases mapped memory **/
            ~DArena();

            /** move-assignment **/
            DArena & operator=(DArena && other);

            ///@}

            /** @defgroup mm-arena-methods **/
            ///@{

            size_type reserved() const noexcept { return hi_ - lo_; }
            size_type allocated() const noexcept { return free_ - lo_; }
            size_type committed() const noexcept { return committed_z_; }
            size_type available() const noexcept { return limit_ - free_; }

            bool contains(const void * addr) const noexcept { return (lo_ <= addr) && (addr < hi_); }

            /** obtain uncommitted contiguous memory range comprising
             *  a whole multiple of @p hugepage_z bytes, of at least size @p req_z,
             *  aligned on a @p hugepage_z boundary
             **/
            static range_type map_aligned_range(size_type req_z, size_type hugepage_z);

            /** true if arena is mapped i.e. has a reserved address range **/
            bool is_mapped() const noexcept { return (lo_ != nullptr) && (hi_ != nullptr); }

            /** get header from allocated object address **/
            header_type * obj2hdr(void * obj) noexcept;

            /** report alloc book-keeping info for allocation at @p mem
             *
             *  Require:
             *  1. @p mem is address returned by allocation on this arena
             *     i.e. by @ref IAllocator_DArena::alloc() or @ref IAllocator_DArena::alloc_super()
             *  2. @p mem has not been invalidated since it was allocated
             *     i.e. by call to @ref DArena::clear
             *
             *  Note: non-const, may stash error details
             **/
            AllocInfo alloc_info(value_type mem) noexcept;

            /** discard all allocated memory, return to empty state
             *  Promise:
             *  - committed memory unchanged
             *  - available memory = committed memory
             **/
            void clear() noexcept;

            ///@}

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
            AllocError last_error_;

            ///@}
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end DArena.hpp */
