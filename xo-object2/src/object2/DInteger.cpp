/** @file DInteger.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DInteger.hpp"
#include <xo/indentlog/print/pretty.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::facet::typeseq;
    using xo::print::ppdetail_atomic;

    namespace scm {
        DInteger *
        DInteger::_box(obj<AAllocator> mm, long x)
        {
            void * mem = mm.alloc(typeseq::id<DInteger>(),
                                  sizeof(DInteger));

            return new (mem) DInteger(x);
        }

        bool
        DInteger::pretty(const ppindentinfo & ppii) const
        {
            return ppdetail_atomic<long>::print_pretty(ppii, value_);
        }

        size_t
        DInteger::shallow_size() const noexcept
        {
            return sizeof(DInteger);
        }

        DInteger *
        DInteger::shallow_copy(obj<AAllocator> mm) const noexcept
        {
            DInteger * copy = (DInteger *)mm.alloc_copy((std::byte *)this);

            if (copy)
                *copy = *this;

            return copy;
        }

        size_t
        DInteger::forward_children(obj<ACollector>) noexcept
        {
            return shallow_size();
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end DInteger.cpp */
