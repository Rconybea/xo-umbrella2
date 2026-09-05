/** @file ATop.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "typeseq.hpp"

namespace xo {
    namespace facet {
        using Copaque = const void *;
        using Opaque = void *;

        /** @class ATop
         *  @brief Abstract facet demanding nothing beyond the fomo-mandatory members
         *
         *  Top of the facet lattice: every representation satisfies ATop, because
         *  ATop asks for nothing fomo does not already require.  Every AFoo inherits
         *  it, so obj<AFoo> narrows to obj<ATop> without a conversion.
         *
         *  Use it where a fomo object must be held with its interface unspecified --
         *  a gc root slot, say.  The runtime representation is still identifiable
         *  (@ref _typeseq) and destroyable (@ref _drop), and any other facet is
         *  recoverable by rotation: FacetRegistry::impl_for<AFoo>(x._typeseq()).
         *
         *  NB ATop is the ONLY base a facet ever has.  Facets are combined by
         *  rotation, not inheritance, which keeps this a single non-virtual chain --
         *  load-bearing for @ref _has_null_vptr (vptr at offset 0) and for the
         *  static_cast<AFoo*> in generated handles.
         *
         *  NB no virtual destructor, matching the other facets: interface instances
         *  have static storage duration (FacetRegistry::register_impl) and are never
         *  deleted through a base pointer.
         **/
        class ATop {
        public:
            /** @defgroup facet-atop-type-traits ATop type traits **/
            ///@{
            /** sequence number identifying a datatype **/
            using typeseq = xo::facet::typeseq;
            ///@}

            /** @defgroup facet-atop-methods ATop methods **/
            ///@{

            /** An uninitialized facet instance will have zero vtable pointer
             *  (per {linux,osx} abi).
             *  Use case for this is narrow.
             *  We go to some lengths to avoid null vtable pointers.
             *  For example obj<AFacet> will have non-null vtable (via IFacet_Any)
             *  with all methods terminating.
             **/
            bool _has_null_vptr() const noexcept {
                return (*reinterpret_cast<const void * const *>(this) == nullptr);
            }
            /** RTTI: unique id# for actual runtime data representation **/
            virtual typeseq _typeseq() const noexcept = 0;
            /** destroy instance @p d. Calls c++ destructor for actual runtime type.
             *  does not recover memory.
             **/
            virtual void _drop(Opaque d) const noexcept = 0;
            ///@}
        }; /*ATop*/
    } /*namespace facet*/
} /*namespace xo*/

/* end ATop.hpp */
