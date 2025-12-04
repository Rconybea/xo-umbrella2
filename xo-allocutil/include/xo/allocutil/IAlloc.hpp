/** @file IAlloc.hpp
 *
 *  @author: Roland Conybeare, Jul 2025
 **/

#pragma once

#include "gc_allocator_traits.hpp"
#include <memory>
#include <cstdint>

namespace xo {
    template <typename T>
    using up = std::unique_ptr<T>;

    class IObject;
    class Object;

    namespace gc {
        /** @class IAllocator
         *  @brief Abstract API for allocation interface
         *
         *  Garbage collector support methods:
         *  - checkpoint()
         *  - assign_member()
         *  - alloc_gc_copy()
         *
         *  See class GC for copying incremental collector.
         *  See class ArenaAlloc for arena allocator
         *
         *  In either case deallocation is trivial
         **/
        class IAlloc {
        public:
            /** type-erased allocator **/
            using value_type = std::byte;
            using pointer_type = std::byte *;
            using size_type = std::size_t;
            /** traits: see gc_allocator_traits<IAlloc>
             *
             *  want Object here (not IObject) to get Object::_forward_to()
             **/
            using gc_object_interface = xo::Object;
            using has_incremental_gc_interface = std::true_type;
            using has_trivial_deallocate = std::true_type;

        public:
            virtual ~IAlloc() {}

            /** word size for alignment **/
            static constexpr uint32_t c_alloc_alignment = sizeof(std::uintptr_t);

            static inline std::uint32_t alloc_padding(std::size_t z) {
                constexpr uint32_t c_bpw = c_alloc_alignment;

                /* round up to multiple of c_bpw, but map 0 -> 0
                 * (table assuming c_bpw==8)
                 *
                 *   z%c_bpw   dz
                 *   ------------
                 *         0    0
                 *         1    7
                 *         2    6
                 *        ..   ..
                 *         7    1
                 */
                std::uint32_t dz = (c_bpw - (z % c_bpw)) % c_bpw;
                z += dz;

                return dz;
            }

            static inline std::size_t with_padding(std::size_t z) {
                return z + alloc_padding(z);
            }

            // ----- std::allocator interface -----

            std::byte * allocate(std::size_t n) {
                return this->alloc(n);
            }

            /** std::allocator locality hint. Not used for now **/
            std::byte * allocate(std::size_t n, const void * hint) {
                (void)hint;
                return this->alloc(n);
            }

            /** deallocation trivial for IAlloc **/
            void deallocate(std::byte *, std::size_t) {}

            // ----- IAlloc methods -----

            /** optional name for this allocator; labelling for diagnostics **/
            virtual const std::string & name() const = 0;
            /** allocator size in bytes (up to reserved limit)
             *  Includes unallocated mmeory
             **/
            virtual std::size_t size() const = 0;
            /** committed size in bytes **/
            virtual std::size_t committed() const = 0;
            /** number of unallocated bytes available (up to soft limit)
             *  from this allocator
             **/
            virtual std::size_t available() const = 0;
            /** number of bytes allocated from this allocator **/
            virtual std::size_t allocated() const = 0;
            /** true iff pointer x comes from this allocator **/
            virtual bool contains(const void * x) const = 0;
            /** true iff object at address @p x was allocated by this allocator,
             *  and before checkpoint
             **/
            virtual bool is_before_checkpoint(const void * x) const = 0;
            /** number of bytes allocated before @ref checkpoint **/
            virtual std::size_t before_checkpoint() const = 0;
            /** number of bytes allocated since @ref checkpoint **/
            virtual std::size_t after_checkpoint() const = 0;
            /** @return true iff debug logging enabled **/
            virtual bool debug_flag() const = 0;

            /** remember allocator state.  All currently-allocated addresses xo
             *  will satisfy is_before_checkpoint(x).  Subsequent allocations x
             *  will fail is_before_checkpoint(x), until checkpoint superseded
             *  by @ref clear or another call to @ref checkpoint
             **/
            virtual void checkpoint() = 0;

            /** allocate @p z bytes of memory. returns pointer to first address **/
            virtual std::byte * alloc(std::size_t z) = 0;
            /** reset allocator to empty state. **/
            virtual void clear() = 0;

            // ----- GC-specific methods -----

            /** true iff this allocator owns object at address @p src.
             *  Use to assist Object::_shallow_move
             **/
            virtual bool check_owned(IObject * /*src*/) const { return false; }
            /** true iff object at address @p src must move as part of
             *  in-progress collection phase
             **/
            virtual bool check_move(IObject * /*src*/) const { return false; }
            /** write barrier for collector.  perform assignment
             *    @code
             *    *lhs = rhs
             *    @endcode
             *  plus additional book keeping if needed (e.g. in @ref GC)
             *  Default implementation just does the assignment.
             **/
            virtual void assign_member(IObject * /*parent*/,
                                       IObject ** lhs,
                                       IObject * rhs) { *lhs = rhs; }
            /** if GC: evacuate @p *lhs and replace with forwarding pointer **/
            virtual void forward_inplace(IObject ** lhs) { (void)lhs; }
            /** allocate @p z bytes for copy of object at @p src.
             *  Only used in @ref GC.  Default implementation asserts and returns nullptr
             **/
            virtual std::byte * alloc_gc_copy(std::size_t /*z*/, const void * /*src*/) {
                // LCOV_EXCL_START
                //assert(false);
                return nullptr;
                // LCOV_EXCL_STOP
            }
        };

        /** allocator wrapper (in the style of std::allocator)
         **/
        template <typename T>
        struct allocator {
        public:
            using value_type = T;
            using pointer = T *;
            using const_pointer = const T *;
            using reference = T &;
            using const_reference = const T &;
            using size_type = IAlloc::size_type;
            using difference_type = std::ptrdiff_t;

            using gc_object_interface = IAlloc::gc_object_interface;
            using has_incremental_gc_interface = IAlloc::has_incremental_gc_interface;

            /** rebind is for typed allocators. since IAlloc is untyped,
             *  we want degenerate version
             **/
            template <typename U>
            struct rebind {
                using other = allocator<U>;
            };

        public:
            explicit allocator(IAlloc * mm) : mm_{mm} {}

            allocator(const allocator &) = default;
            allocator & operator=(const allocator &) = default;

            template <typename U>
            allocator(const allocator<U> & other) : mm_{other.mm_} {}

            pointer allocate(size_type n) {
                std::byte * raw = mm_->allocate(n * sizeof(T));

                return reinterpret_cast<pointer>(raw);
            }

            void deallocate(pointer p, size_type n) {
                std::byte * raw = reinterpret_cast<std::byte *>(p);

                mm_->deallocate(raw, n * sizeof(T));
            }

            // optional construct, destroy (but allocator_traits provides defaults)

            /** required! otherwise allocator<T>, allocator<U> with the same IAlloc*
             *  would be considered to own disjoin memory addresses
             **/
            template <typename U>
            bool operator==(const allocator<U> & other) const noexcept {
                return mm_ == other.mm_;
            }

        public:
            IAlloc * mm_ = nullptr;
        };

    } /*namespace gc*/

    class MMPtr {
    public:
        explicit MMPtr(gc::IAlloc * mm) : mm_{mm} {}

        gc::IAlloc * mm_ = nullptr;
    };
} /*namespace xo*/

inline void * operator new (std::size_t z, const xo::MMPtr & mmp) {
    return mmp.mm_->alloc(z);
}

//inline void operator delete (void * p, const MMPtr & mmp) {
//    mmp.mm_->free(reinterpret_cast<std::byte *>(p));
//}

inline void * operator new[] (std::size_t z, const xo::MMPtr & mmp) {
    return mmp.mm_->alloc(z);
}

//inline void operator delete[] (void * p, const MMPtr & mmp) {
//    mmp.mm_->free(reinterpret_cast<std::byte *>(p));
//}


/* end IAlloc.hpp */
