/** @file ACollector2.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Collector2.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Collector2.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/generation.hpp>
#include <xo/alloc2/role.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

class AGCObject;
// more pretext here..

namespace xo {
namespace mm {

using Copaque = const void *;
using Opaque = void *;

/**
A collector must also suppose the @ref AAllocator facet, see also
**/
class ACollector2 {
public:
    /** @defgroup mm-collector2-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /** allocation size type **/
    using size_type = std::size_t;
    ///@}

    /** @defgroup mm-collector2-methods **/
    ///@{
    // const methods
    /** An uninitialized ACollector2 instance will have zero vtable pointer (per {linux,osx} abi).
     *  Use case for this is narrow. We go to some lengths to avoid null vtable pointers. For example
     *  obj<AFacet> will have non-null vtable (via IFacet_Any) with all methods terminating.
     **/
    bool _has_null_vptr() const noexcept { return *reinterpret_cast<const void * const *>(this) == nullptr; }
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;
    /** memory in use for this collector **/
    virtual size_type allocated(Copaque data, Generation g, role r)  const  noexcept = 0;
    /** memory committed for this collector **/
    virtual size_type committed(Copaque data, Generation g, role r)  const  noexcept = 0;
    /** address space reserved for this collector **/
    virtual size_type reserved(Copaque data, Generation g, role r)  const  noexcept = 0;
    /** true if gc responsible for data at @p addr, and data belongs to role @p r **/
    virtual bool contains(Copaque data, role r, const void * addr)  const  noexcept = 0;
    /** true iff gc-aware object of type @p tseq is installed in this collector **/
    virtual bool is_type_installed(Copaque data, typeseq tseq)  const  noexcept = 0;

    // nonconst methods
    /** install interface @p iface for representation with typeseq @p tseq
in collector @p d.

The type AGCObject_Any here is misleading.
Will have been replaced by an instance of
  @c AGCObject_Xfer<DFoo,AGCObject_DFoo> for some @c DFoo
in which case calls through @c std::launder(&iface)
will properly act on @c DFoo.

Return false if installation fails (e.g. memory exhausted) **/
    virtual bool install_type(Opaque data, const AGCObject & iface)  = 0;
    /** add gc root with address @p p_root. gc will preserve subgraph at this address **/
    virtual void add_gc_root_poly(Opaque data, obj<AGCObject> * p_root)  = 0;
    /** remove gc root with address @p p_root. Reverse effect of prior add_gc_root_poly call **/
    virtual void remove_gc_root_poly(Opaque data, obj<AGCObject> * p_root)  = 0;
    /** Request immediate collection.
  1. if collection is enabled, immediately collect all generations
     up to (but not including) g
  2. may nevertheless escalate to older generations,
     depending on collector state.
  3. if collection is currently disabled,
     collection will trigger the next time gc is enabled.
 **/
    virtual void request_gc(Opaque data, Generation upto)  = 0;
    /** Assign pointer @p p_lhs to destination @p rhs, within parent allocation @p parent

Require: gc not in progress **/
    virtual void assign_member(Opaque data, void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> & rhs)  = 0;
    /** evacuate @p *lhs, that refers to state with interface @p lhs_iface,
to collector @p d's to-space. Replace *lhs_data with forwarding pointer

Require: gc in progress
 **/
    virtual void forward_inplace(Opaque data, AGCObject * lhs_iface, void ** lhs_data)  = 0;
    ///@}
}; /*ACollector2*/

/** Implementation ICollector2_DRepr of ACollector2 for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<ACollector2, DRepr> {
 *        using Impltype = ICollector2_DRepr;
 *    };
 *
 *  then ICollector2_ImplType<DRepr> --> ICollector2_DRepr
 **/
template <typename DRepr>
using ICollector2_ImplType = xo::facet::FacetImplType<ACollector2, DRepr>;

} /*namespace mm*/
} /*namespace xo*/

/* ACollector2.hpp */
