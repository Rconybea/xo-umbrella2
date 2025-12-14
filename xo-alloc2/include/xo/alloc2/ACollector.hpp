/** @file ACollector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "gc/generation.hpp"
#include "gc/role.hpp"

#include <cstdint>
#include <cstddef>

namespace xo {
    namespace mm {
        using Copaque = const void *;
        using Opaque = void *;

        /** @class ACollector
         *  @brief Abstract facet for the XO garbage collector
         *
         *  Collector also supports the @ref AAllocator facet, see also
         **/
        struct ACollector {
            using size_type = std::size_t;

            virtual int32_t _typeseq() const noexcept = 0;

            virtual size_type allocated(Copaque d, generation g, role r) const noexcept = 0;
            virtual size_type reserved(Copaque d, generation g, role r) const noexcept = 0;
            virtual size_type committed(Copaque d, generation g, role r) const noexcept = 0;

            /** install interface @p iface for representation with typeseq @p tseq
             *  in collector @p d.
             *
             *  The type AGCObject_Any here is misleading.
             *  Will have been replaced by an instance of
             *    @c AGCObject_Xfer<DFoo,AGCObject_DFoo> for some @c DFoo
             *  in which case calls through @c std::launder(&iface)
             *  will properly act on @c DFoo.
             **/
            virtual void install_type(Opaque d, int32_t tseq, AGCObject_Any & iface);
            virtual void add_gc_root(Opaque d, int32_t tid, Opaque * root) = 0;
        };
    }

} /*namespace xo*/
