/* file IAlloc.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include <memory>
#include <cstdint>

namespace xo {
    template <typename T>
    using up = std::unique_ptr<T>;

    namespace gc {
        /** @class IAllocator
         *  @brief memory allocation interface with limited garbaga collector support
         **/
        class IAlloc {
        public:
            virtual ~IAlloc() {}

            /** allocator size in bytes (up to soft limit).
             *  Includes unallocated mmeory
             **/
            virtual std::size_t size() const = 0;
            /** number of unallocated bytes available (up to soft limit)
             *  from this allocator
             **/
            virtual std::size_t available() const = 0;
            /** number of bytes allocated from this allocator **/
            virtual std::size_t allocated() const = 0;
            /** true iff object at address @p x was allocated by this allocator,
             *  and before checkpoint
             **/
            virtual bool is_before_checkpoint(const std::uint8_t * x) const = 0;
            /** number of bytes allocated before @ref checkpoint **/
            virtual std::size_t before_checkpoint() const = 0;
            /** number of bytes allocated since @ref checkpoint **/
            virtual std::size_t after_checkpoint() const = 0;

            /** reset allocator to empty state. **/
            virtual void clear() = 0;
            /** remember allocator state.  All currently-allocated addresses x
             *  will satisfy is_before_checkpoint(x).  Subsequent allocations x
             *  will fail is_before_checkpoint(x), until checkpoint superseded
             *  by @ref clear or another call to @ref checkpoint
             **/
            virtual void checkpoint() = 0;
            /** allocate @p z bytes of memory. returns pointer to first address **/
            virtual std::uint8_t * alloc(std::size_t z) = 0;
        };
    } /*namespace gc*/
} /*namespace xo*/


/* end IAlloc.hpp */
