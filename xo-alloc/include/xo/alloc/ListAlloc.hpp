/* file ListAlloc.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "IAlloc.hpp"
#include <memory>
#include <cstdint>

namespace xo {
    namespace gc {
        /** GC-compatible allocator using a linked list of buckets.
         *
         *  GC Support:
         *  - reserved memory, released after call to @ref release_redline_memory.
         *
         *  TODO: reserve address space using mmap,
         *        but don't commit until alloc requires it.
         **/
        class ListAlloc : public IAlloc {
        public:
            ListAlloc(LinearAlloc* hd,
                      std::size_t cz, std::size_t nz; std::size_tz,
                      LinearAlloc* marked, bool use_redline,
                      bool redlined_flag, OnEmptyFn on_overflow);
            ~ListAlloc();

            static up<ListAlloc> make(std::size_t cz, std::size_t nz,
                                      OnEmptyFn on_overflow);

        private:
            std::size_t start_z_ = 0;
            LinearAlloc* hd_ = nullptr;
            std::size_t current_z_ = 0;;
            std::size_t next_z_ = 0;;
            std::size_t total_z_ = 0;
            bool use_redline_ = false;
            bool redlined_flag_ = false;

        };
    } /*namespace gc*/
} /*namespace xo*/


/* end ListAlloc.hpp */
