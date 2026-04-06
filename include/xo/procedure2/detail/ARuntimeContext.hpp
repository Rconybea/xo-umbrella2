/** @file ARuntimeContext.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/RuntimeContext.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/RuntimeContext.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/stringtable2/StringTable.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/Collector.hpp>
#include <xo/arena/MemorySizeInfo.hpp>
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
    /** xo garbage collector **/
    using ACollector = xo::mm::ACollector;
    /** function to visit memory pools **/
    using MemorySizeVisitor = xo::mm::MemorySizeVisitor;
    ///@}

    /** @defgroup scm-runtimecontext-methods **/
    ///@{
    // const methods
    /** An uninitialized ARuntimeContext instance will have zero vtable pointer (per {linux,osx} abi).
     *  Use case for this is narrow. We go to some lengths to avoid null vtable pointers. For example
     *  obj<AFacet> will have non-null vtable (via IFacet_Any) with all methods terminating.
     **/
    bool _has_null_vptr() const noexcept { return *reinterpret_cast<const void * const *>(this) == nullptr; }
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;
    /** default allocator to use for objects **/
    virtual obj<AAllocator> allocator(Copaque data)  const  noexcept = 0;
    /** collector facet for allocator. If non-null, same data pointer as allocator **/
    virtual obj<ACollector> collector(Copaque data)  const  noexcept = 0;
    /** last-resort allocator for erros. e.g. regular allocator exhausted **/
    virtual obj<AAllocator> error_allocator(Copaque data)  const  noexcept = 0;
    /** stringtable for unique symbols **/
    virtual StringTable * stringtable(Copaque data)  const  noexcept = 0;
    /** invoke visitor for each distinct memory pool **/
    virtual void visit_pools(Copaque data, MemorySizeVisitor visitor)  const = 0;

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
