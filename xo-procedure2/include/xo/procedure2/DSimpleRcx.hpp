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
            using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

        public:
            DSimpleRcx(obj<AAllocator> mm, StringTable * st)
                : allocator_{mm}, stringtable_{st} {}

            obj<AAllocator> allocator() const noexcept { return allocator_; }
            StringTable * stringtable() const noexcept { return stringtable_; }
            void visit_pools(const MemorySizeVisitor & visitor) const;

        private:
            obj<AAllocator> allocator_;
            StringTable * stringtable_ = nullptr;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DSimpleRcx.hpp */
