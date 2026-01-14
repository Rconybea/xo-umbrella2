/** @file AGCObject.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/GCObject.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/GCObject.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/alloc2/Allocator.hpp>
#include <xo/gc/Collector.hpp>
#include <cstdint>
#include <cstddef>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

namespace xo { namespace mm { struct ACollector; }}

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
    /** type for an amount of memory **/
    using size_type = std::size_t;
    ///@}

    /** @defgroup mm-gcobject-methods **/
    ///@{
    // const methods
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** memory consumption for this instance **/
    virtual size_type shallow_size(Copaque data)  const  noexcept = 0;
    /** copy instance using allocator **/
    virtual Opaque shallow_copy(Copaque data, obj<AAllocator> mm)  const  noexcept = 0;

    // nonconst methods
    /** during GC: forward immdiate children **/
    virtual size_type forward_children(Opaque data, obj<ACollector> gc)  const  noexcept = 0;
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