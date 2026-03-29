/** @file DSimpleRcx.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/stringtable2/StringTable.hpp>
#include <xo/alloc2/Allocator.hpp>

namespace xo {
    namespace scm {

        /** @brief Minimal runtime context.
         *
         *  Minimal runtime context provides an allocator,
         *  and nothing more.
         **/
        class DSimpleRcx {
        public:
            using AAllocator = xo::mm::AAllocator;
            using ACollector = xo::mm::ACollector;
            using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

        public:
            DSimpleRcx(obj<AAllocator> mm, obj<AAllocator> error_mm, StringTable * st)
            : allocator_{mm}, error_allocator_{error_mm},
              stringtable_{st} {}

            obj<AAllocator> allocator() const noexcept { return allocator_; }
            obj<ACollector> collector() const noexcept;
            obj<AAllocator> error_allocator() const noexcept { return error_allocator_; }
            StringTable * stringtable() const noexcept { return stringtable_; }
            void visit_pools(const MemorySizeVisitor & visitor) const;

        private:
            obj<AAllocator> allocator_;
            obj<AAllocator> error_allocator_;
            StringTable * stringtable_ = nullptr;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DSimpleRcx.hpp */
