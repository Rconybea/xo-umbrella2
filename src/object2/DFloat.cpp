/** @file DFloat.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "DFloat.hpp"
#include <xo/indentlog/print/pretty.hpp>

namespace xo {
    using xo::facet::typeseq;
    using xo::print::ppdetail_atomic;
    using std::size_t;

    namespace scm {
        DFloat *
        DFloat::_box(obj<AAllocator> mm, double x)
        {
            void * mem = mm.alloc(typeseq::id<DFloat>(),
                                  sizeof(DFloat));

            return new (mem) DFloat(x);
        }

        bool
        DFloat::pretty(const ppindentinfo & ppii) const
        {
            return ppdetail_atomic<double>::print_pretty(ppii, value_);
        }

        size_t
        DFloat::shallow_size() const noexcept
        {
            return sizeof(DFloat);
        }

        DFloat *
        DFloat::shallow_copy(obj<AAllocator> mm) const noexcept
        {
            DFloat * copy = (DFloat *)mm.alloc_copy((std::byte *)this);

            if (copy)
                *copy = *this;

            return copy;
        }

        size_t
        DFloat::forward_children(obj<ACollector>) noexcept
        {
            return shallow_size();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DFloat.cpp */
