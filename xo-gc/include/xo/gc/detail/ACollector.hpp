/** @file ACollector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "IGCObject_Any.hpp"

#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>
#include <xo/facet/obj.hpp>

#include "generation.hpp"
#include "role.hpp"

#include <cstdint>
#include <cstddef>

namespace xo {
    namespace mm {
        using Copaque = const void *;
        using Opaque = void *;

        struct IGCObject_Any; // see IGCObject_Any.hpp

        /** @class ACollector
         *  @brief Abstract facet for the XO garbage collector
         *
         *  A collector implementation will also support the @ref AAllocator facet, see also
         **/
        struct ACollector {
            using size_type = std::size_t;

            virtual int32_t _typeseq() const noexcept = 0;

            virtual size_type allocated(Copaque d,
                                        generation g, role r) const noexcept = 0;
            virtual size_type  reserved(Copaque d,
                                        generation g, role r) const noexcept = 0;
            virtual size_type committed(Copaque d,
                                        generation g, role r) const noexcept = 0;

            /** install interface @p iface for representation with typeseq @p tseq
             *  in collector @p d.
             *
             *  The type AGCObject_Any here is misleading.
             *  Will have been replaced by an instance of
             *    @c AGCObject_Xfer<DFoo,AGCObject_DFoo> for some @c DFoo
             *  in which case calls through @c std::launder(&iface)
             *  will properly act on @c DFoo.
             **/
            virtual void install_type(Opaque d, int32_t tseq, IGCObject_Any & iface) = 0;
            virtual void add_gc_root(Opaque d, int32_t tseq, Opaque * root) = 0;

            /** evacuate @p *lhs to to-space and replace with forwarding pointer
             *  Require: gc in progress
             **/
            virtual void forward_inplace(Opaque d, obj<AGCObject> * lhs) = 0;
        };
    }

} /*namespace xo*/
