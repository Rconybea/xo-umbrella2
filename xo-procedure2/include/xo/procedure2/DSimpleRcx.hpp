/** @file DSimpleRcx.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

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

        public:
            DSimpleRcx(obj<AAllocator> mm) : allocator_{mm} {}

            obj<AAllocator> allocator() const noexcept { return allocator_; }

        private:
            obj<AAllocator> allocator_;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DSimpleRcx.hpp */
