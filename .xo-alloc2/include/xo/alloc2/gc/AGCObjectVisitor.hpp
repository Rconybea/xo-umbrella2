/** @file AGCObjectVisitor.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/GCObjectVisitor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/GCObjectVisitor.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/alloc2/Generation.hpp>
#include <xo/alloc2/role.hpp>
#include <xo/alloc2/VisitReason.hpp>
#include <xo/arena/AllocInfo.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

// see GCObject.hpp, also in xo-alloc2/
namespace xo { namespace mm { class AGCObject; }}
namespace xo { namespace mm { class AllocInfo; }}
namespace xo { namespace mm { class Generation; }}

namespace xo {
namespace mm {

using Copaque = const void *;
using Opaque = void *;

/**
Visit a gc-aware object. Visitor can traverse and update child pointers in-place.
**/
class AGCObjectVisitor {
public:
    /** @defgroup mm-gcobjectvisitor-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    ///@}

    /** @defgroup mm-gcobjectvisitor-methods **/
    ///@{
    // const methods
    /** An uninitialized AGCObjectVisitor instance will have zero vtable pointer (per {linux,osx} abi).
     *  Use case for this is narrow. We go to some lengths to avoid null vtable pointers. For example
     *  obj<AFacet> will have non-null vtable (via IFacet_Any) with all methods terminating.
     **/
    bool _has_null_vptr() const noexcept { return *reinterpret_cast<const void * const *>(this) == nullptr; }
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;
    /** allocation metadata for gc-aware data at address @p gco.
@p gco must be the result of a call to collector's alloc() function
note: load-bearing for xo-gc/MutationLogStore **/
    virtual AllocInfo alloc_info(Copaque data, void * addr)  const = 0;
    /** generation to which pointer @p addr belongs, given role @p r;
sentinel if @p addr is not owned by collector.
note: load-bearing for xo-gc/MutationLogStore **/
    virtual Generation generation_of(Copaque data, Role r, const void * addr)  const  noexcept = 0;

    // nonconst methods
    /** allocate copy of source object at address @p src.
Source must be owned by this collector.
Increments object age **/
    virtual void * alloc_copy(Opaque data, std::byte * src)  const = 0;
    /** visit child of a gc-aware object. May update child in-place! **/
    virtual void visit_child(Opaque data, VisitReason reason, AGCObject * iface, void ** pp_data)  const  noexcept = 0;
    ///@}
}; /*AGCObjectVisitor*/

/** Implementation IGCObjectVisitor_DRepr of AGCObjectVisitor for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AGCObjectVisitor, DRepr> {
 *        using Impltype = IGCObjectVisitor_DRepr;
 *    };
 *
 *  then IGCObjectVisitor_ImplType<DRepr> --> IGCObjectVisitor_DRepr
 **/
template <typename DRepr>
using IGCObjectVisitor_ImplType = xo::facet::FacetImplType<AGCObjectVisitor, DRepr>;

} /*namespace mm*/
} /*namespace xo*/

/* AGCObjectVisitor.hpp */
