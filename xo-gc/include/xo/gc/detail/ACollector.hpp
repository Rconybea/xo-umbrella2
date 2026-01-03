/** @file ACollector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

//#include "IGCObject_Any.hpp"

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

        struct AGCObject;
        struct IGCObject_Any; // see IGCObject_Any.hpp

        /** @class ACollector
         *  @brief Abstract facet for the XO garbage collector
         *
         *  A collector implementation will also support the @ref AAllocator facet, see also
         **/
        struct ACollector {
            using typeseq = xo::facet::typeseq;
            using size_type = std::size_t;

            virtual typeseq _typeseq() const noexcept = 0;

            virtual size_type allocated(Copaque d,
                                        generation g, role r) const noexcept = 0;
            virtual size_type  reserved(Copaque d,
                                        generation g, role r) const noexcept = 0;
            virtual size_type committed(Copaque d,
                                        generation g, role r) const noexcept = 0;
            virtual bool is_type_installed(Copaque d,
                                           typeseq tseq) const noexcept = 0;

            /** install interface @p iface for representation with typeseq @p tseq
             *  in collector @p d.
             *
             *  The type AGCObject_Any here is misleading.
             *  Will have been replaced by an instance of
             *    @c AGCObject_Xfer<DFoo,AGCObject_DFoo> for some @c DFoo
             *  in which case calls through @c std::launder(&iface)
             *  will properly act on @c DFoo.
             *
             *  Return false if installation fails (e.g. memory exhausted)
             **/
            virtual bool install_type(Opaque d, const AGCObject & iface) = 0;
            virtual void add_gc_root(Opaque d, int32_t tseq, Opaque * root) = 0;

            /** Request immediate collection.
             *  1. if collection is enabled, immediately collect all generations
             *     up to (but not including) g
             *  2. may nevertheless escalate to older generations,
             *     depending on collector state.
             *  3. if collection is currently disabled,
             *     collection will trigger the next time gc is enabled.
             **/
            virtual void request_gc(Opaque d, generation upto) = 0;

            /** evacuate @p *lhs, that refers to state with interface @p lhs_iface,
             *  to collector @p d's to-space. Replace *lhs_data with forwarding pointer
             *
             *  Require: gc in progress
             **/
            virtual void forward_inplace(Opaque d, AGCObject * lhs_iface, void ** lhs_data) = 0;
        };
    }

} /*namespace xo*/
