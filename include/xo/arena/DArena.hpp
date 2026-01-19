/** @file DArena.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "ArenaConfig.hpp"
#include "AllocError.hpp"
#include "AllocInfo.hpp"
#include <xo/reflectutil/typeseq.hpp>

namespace xo {
    namespace mm {
        struct DArenaIterator; // see DArenaIterator.hpp

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
            using header_type = AllocHeader;
            /** integer identifying a type (see xo::facet::typeid<T>()) **/
            using typeseq = xo::reflect::typeseq;

            /** @brief mode argument for @ref _alloc **/
            enum class alloc_mode : uint8_t {
                /** ordinary alloc. Most common mode **/
                standard,
                /** begin a sequence of suballocs that share a single alloc header **/
                super,
                /** make a subsidiary allocation on behalf of a preceding super alloc.
                 *  Will be followed by at least one more suballoc call.
                 **/
                sub_incomplete,
                /** make a subsidiary allocation that completes preceding super alloc. **/
                sub_complete,
            };

            /** @brief Checkpoint for unwinding arena state **/
            struct Checkpoint {
                explicit Checkpoint(std::byte * x) : free_{x} {}
                std::byte * free_;
            };

            ///@}

            /** @defgroup mm-arena-ctors arena constructors and destructors **/
            ///@{

            /** create arena per configuration @p cfg. **/
            static DArena map(const ArenaConfig & cfg);

            /** null ctor **/
            DArena() = default;
            /** ctor from already-mapped (but not committed) address range **/
            DArena(const ArenaConfig & cfg,
                   size_type page_z,
                   size_type arena_align_z,
                   value_type lo,
                   value_type hi);
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

            /** Reserved memory, in bytes. This is the maximum size of this arena. **/
            size_type reserved() const noexcept { return hi_ - lo_; }
            /** Allocated memory in bytes: memory consumed by allocs from this arena,
             *  including administrative overhead (alloc headers + guard bytes)
             **/
            size_type allocated() const noexcept { return free_ - lo_; }
            /** Committed memory in bytes: amount of memory actually backed by physical memory **/
            size_type committed() const noexcept { return committed_z_; }
            /** Available committed memory.
             *  This is the amount of memory guaranteed to be usable for future allocs from this arena.
             **/
            size_type available() const noexcept { return limit_ - free_; }
            /** VM page size for this arena (likely 4KB) **/
            size_type page_z() const noexcept { return page_z_; }
            /** Last error encountered by this arena **/
            const AllocError & last_error() const noexcept { return last_error_; }

            /** True iff address @p addr is owned by this arena,
             *  i.e. falls within [@ref lo_, @ref hi_)
             **/
            bool contains(const void * addr) const noexcept { return (lo_ <= addr) && (addr < hi_); }

            /** true if arena is mapped i.e. has a reserved address range **/
            bool is_mapped() const noexcept { return (lo_ != nullptr) && (hi_ != nullptr); }

            /** @ret iterator pointing to the first allocation in this arena **/
            DArenaIterator begin() const noexcept;
            /** @ret iterator pointing to just after the last allocation in this arena **/
            DArenaIterator end() const noexcept;

            /** @ret header for first allocation in this arena **/
            AllocHeader * begin_header() const noexcept;
            /** @ret location of header for next (not yet performed!)
             *  allocation in this arena
             **/
            AllocHeader * end_header() const noexcept;

            /** get header from allocated object address **/
            header_type * obj2hdr(void * obj) noexcept;

            /** report alloc book-keeping info for allocation at @p mem
             *
             *  Require:
             *  1. @p mem is address returned by allocation on this arena
             *     i.e. by @ref IAllocator_DArena::alloc() or
             *     @ref IAllocator_DArena::alloc_super()
             *  2. @p mem has not been invalidated since it was allocated
             *     i.e. by call to @ref DArena::clear
             *
             *  Note: non-const, may stash error details
             **/
            AllocInfo alloc_info(value_type mem) const noexcept;

            /** allocate at least @p z bytes of memory.
             *  Return nullptr and capture error if unable to satisfy request.
             *  May expand committed memory, as long as resulting committed size
             *  is no larger than reserved size
             **/
            value_type alloc(typeseq t, size_type z);

            /** when store_header_flag enabled:
             *   like alloc(), but combine memory consumed by this alloc
             *   plus following consecutive sub_alloc()'s into a single header.
             *  otherwise equivalent to alloc()
             **/
            value_type super_alloc(typeseq t, size_type z);

            /** when store_header_flag enabled:
             *  follow preceding super_alloc() by one or more sub_allocs().
             *  accumulate total allocated size (including padding) into
             *  single header. All sub_allocs() except the last must set
             *  @p complete_flag to false. The last sub_alloc() must set
             *  @p complete_flag to true.
             **/
            value_type sub_alloc(size_type z, bool complete_flag);

            /** alloc copy of @p src **/
            value_type alloc_copy(value_type src);

            /** capture error information: advance error count + set last_error **/
            void capture_error(error err,
                               size_type target_z = 0) const;

            /** alloc driver. shared by alloc(), super_alloc(), sub_alloc() **/
            value_type _alloc(std::size_t req_z,
                              alloc_mode mode,
                              typeseq tseq,
                              uint32_t age);

            /** expand committed space in arena @p d
             *  to size at least @p z
             *  In practice will round up to a multiple of @ref page_z_.
             **/
            bool expand(size_type z) noexcept;

            /** create initial guard **/
            void establish_initial_guard() noexcept;

            /** checkpoint arena state.  Revert to the same state with
             *  @ref rstore
             **/
            Checkpoint checkpoint() noexcept { return Checkpoint(free_); }

            /** restore arena state to previously-established checkpoint **/
            void restore(Checkpoint ckp) noexcept { free_ = ckp.free_; }

            /** discard all allocated memory, return to empty state
             *  Promise:
             *  - committed memory unchanged
             *  - available memory = committed memory
             **/
            void clear() noexcept;

            /** swap contents (including configuration) with another arena **/
            void swap(DArena & other) noexcept;

            ///@}

            /** @defgroup mm-arena-instance-vars **/
            ///@{

            /** arena configuration **/
            ArenaConfig config_;

            /** size of a VM page (obtained automatically via getpagesize()). Likely 4k **/
            size_type page_z_ = 0;

            /** alignment for this arena.  In practice will be either page_z_ or cfg.hugepage_z_ **/
            size_type arena_align_z_ = 0;

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

        /** construct a @tparam T instance from arguments @p args
         *  using memory obtained from arena @p ialloc
         **/
        template <typename T,
                  typename... Args>
        static T *
        construct_with(DArena & ialloc, Args&&... args)
        {
            using xo::reflect::typeseq;

            typeseq t = typeseq::id<T>();
            std::byte * mem = ialloc.alloc(t, sizeof(T));

            if (mem)
                return new (mem) T(std::forward<Args>(args)...);

            return nullptr;
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end DArena.hpp */
