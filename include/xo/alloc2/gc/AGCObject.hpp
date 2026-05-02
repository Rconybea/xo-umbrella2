/** @file AGCObject.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/GCObject.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/GCObject.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/alloc2/Allocator_basic.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>
#include <cstdint>
#include <cstddef>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

namespace xo { namespace mm { class ACollector; }}

namespace xo {
namespace mm {

using Copaque = const void *;
using Opaque = void *;

/**
GC hooks for collector-aware data
**/
class AGCObject {
public:
    /** @defgroup mm-gcobject-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /** type for an amount of memory **/
    using size_type = std::size_t;
    /** fomo allocator type **/
    using AAllocator = xo::mm::AAllocator;
    /** fomo collector type **/
    using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
    /** hint arg when navigating object graph **/
    using VisitReason = xo::mm::VisitReason;
    ///@}

    /** @defgroup mm-gcobject-methods **/
    ///@{
    // const methods
    /** An uninitialized AGCObject instance will have zero vtable pointer (per {linux,osx} abi).
     *  Use case for this is narrow. We go to some lengths to avoid null vtable pointers. For example
     *  obj<AFacet> will have non-null vtable (via IFacet_Any) with all methods terminating.
     **/
    bool _has_null_vptr() const noexcept { return *reinterpret_cast<const void * const *>(this) == nullptr; }
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;

    // nonconst methods
    /** move instance using object visitor.
Arguably abusing the word 'visitor' here **/
    virtual Opaque gco_shallow_move(Opaque data, obj<AGCObjectVisitor> gc)  const  noexcept = 0;
    /** Invoke fn.visit_child(iface,data) for each child GCObject pointer.
Context: provides address of data pointer so it can be updated in place
when @p fn invokes garbage collector reentry point **/
    virtual void visit_gco_children(Opaque data, VisitReason reason, obj<AGCObjectVisitor> fn)  const  noexcept = 0;
    ///@}
}; /*AGCObject*/

/** Implementation IGCObject_DRepr of AGCObject for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AGCObject, DRepr> {
 *        using Impltype = IGCObject_DRepr;
 *    };
 *
 *  then IGCObject_ImplType<DRepr> --> IGCObject_DRepr
 **/
template <typename DRepr>
using IGCObject_ImplType = xo::facet::FacetImplType<AGCObject, DRepr>;

} /*namespace mm*/
} /*namespace xo*/

/* AGCObject.hpp */
