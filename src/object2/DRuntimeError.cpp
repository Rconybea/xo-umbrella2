/** @file DRuntimeError.cpp
*
 *  @author Roland Conybeare, Feb 2026
 **/

#include "DRuntimeError.hpp"

namespace xo {
    using xo::mm::AGCObject;
    using xo::facet::typeseq;

    namespace scm {

        DRuntimeError *
        DRuntimeError::_make(obj<AAllocator> mm,
                             DString * src_fn,
                             DString * error_descr)
        {
            void * mem
                = mm.alloc(typeseq::id<DRuntimeError>(),
                           sizeof(DRuntimeError));

            DRuntimeError * err
                = new (mem) DRuntimeError(src_fn, error_descr);

            return err;
        }

        DRuntimeError::DRuntimeError(DString * src_fn,
                                     DString * error_descr) : src_function_{src_fn},
                                                              error_descr_{error_descr}
        {}

        // ----- GCObject facet -----

        std::size_t
        DRuntimeError::shallow_size() const noexcept
        {
            return sizeof(DRuntimeError);
        }

        DRuntimeError *
        DRuntimeError::shallow_copy(obj<AAllocator> mm) const noexcept
        {
            DRuntimeError * copy = (DRuntimeError *)mm.alloc_copy((std::byte *)this);

            if (copy)
                *copy = *this;

            return copy;
        }

        std::size_t
        DRuntimeError::forward_children(obj<ACollector> gc) noexcept
        {
            {
                auto iface = xo::facet::impl_for<AGCObject,DString>();
                gc.forward_inplace(&iface, (void **)(&src_function_));
            }

            {
                auto iface = xo::facet::impl_for<AGCObject,DString>();
                gc.forward_inplace(&iface, (void **)(&error_descr_));
            }

            return this->shallow_size();
        }

        // ----- Printable facet -----

        bool
        DRuntimeError::pretty(const ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct(ppii,
                                             "DRuntimeError");
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DRuntimeError.cpp */

