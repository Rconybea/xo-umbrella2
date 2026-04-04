/** @file DBoolean.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DBoolean.hpp"
#include <xo/indentlog/print/pretty.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::facet::typeseq;
    using xo::print::ppdetail_atomic;

    namespace scm {
        DBoolean *
        DBoolean::_box(obj<AAllocator> mm, bool x)
        {
            void * mem = mm.alloc(typeseq::id<DBoolean>(),
                                  sizeof(DBoolean));

            return new (mem) DBoolean(x);
        }

        bool
        DBoolean::pretty(const ppindentinfo & ppii) const
        {
            return ppdetail_atomic<const char *>::print_pretty
                       (ppii,
                        (value_ ? "true" : "false"));
        }

        size_t
        DBoolean::shallow_size() const noexcept
        {
            return sizeof(DBoolean);
        }

        DBoolean *
        DBoolean::shallow_move(obj<ACollector> gc) noexcept
        {
            return gc.std_copy_for(this);
        }

        size_t
        DBoolean::forward_children(obj<ACollector>) noexcept
        {
            return shallow_size();
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end DBoolean.cpp */
