/** @file ARuntimeContext.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2-claude1/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/RuntimeContext.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/RuntimeContext.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>


namespace xo {
namespace scm {

using Copaque = const void *;
using Opaque = void *;

/**
Runtime application context
**/
class ARuntimeContext {
public:
    /** @defgroup scm-runtimecontext-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /** xo memory allocator **/
    using AAllocator = xo::mm::AAllocator;
    ///@}

    /** @defgroup scm-runtimecontext-methods **/
    ///@{
    // const methods
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** default allocator to use for objects **/
    virtual obj<AAllocator> allocator(Copaque data)  const  noexcept = 0;

    // nonconst methods
    ///@}
}; /*ARuntimeContext*/

/** Implementation IRuntimeContext_DRepr of ARuntimeContext for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<ARuntimeContext, DRepr> {
 *        using Impltype = IRuntimeContext_DRepr;
 *    };
 *
 *  then IRuntimeContext_ImplType<DRepr> --> IRuntimeContext_DRepr
 **/
template <typename DRepr>
using IRuntimeContext_ImplType = xo::facet::FacetImplType<ARuntimeContext, DRepr>;

} /*namespace scm*/
} /*namespace xo*/

/* ARuntimeContext.hpp */