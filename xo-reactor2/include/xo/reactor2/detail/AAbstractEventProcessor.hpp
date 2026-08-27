/** @file AAbstractEventProcessor.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/AbstractEventProcessor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/AbstractEventProcessor.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/stringtable2/DString.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

namespace xo::scm { class DString; }

namespace xo {
namespace reactor {

using Copaque = const void *;
using Opaque = void *;

/**
Trait for data types representing nodes in a computational dataflow graph.
**/
class AAbstractEventProcessor {
public:
    /** @defgroup reactor-abstracteventprocessor-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /**  **/
    using obj_AAbstractEventProcessor = xo::facet::obj<AAbstractEventProcessor>;
    /** string representation **/
    using DString = xo::scm::DString;
    ///@}

    /** @defgroup reactor-abstracteventprocessor-methods **/
    ///@{
    // const methods
    /** An uninitialized AAbstractEventProcessor instance will have zero vtable pointer (per {linux,osx} abi).
     *  Use case for this is narrow. We go to some lengths to avoid null vtable pointers. For example
     *  obj<AFacet> will have non-null vtable (via IFacet_Any) with all methods terminating.
     **/
    bool _has_null_vptr() const noexcept { return *reinterpret_cast<const void * const *>(this) == nullptr; }
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;
    /** Reporting name for this node.  Ideally (but not necessarily) unique **/
    virtual const DString * name(Copaque data)  const  noexcept = 0;

    // nonconst methods
    /** Assign reporting name for this node. **/
    virtual void set_name(Opaque data, const DString * name)  noexcept = 0;
    ///@}
}; /*AAbstractEventProcessor*/

/** Implementation IAbstractEventProcessor_DRepr of AAbstractEventProcessor for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AAbstractEventProcessor, DRepr> {
 *        using Impltype = IAbstractEventProcessor_DRepr;
 *    };
 *
 *  then IAbstractEventProcessor_ImplType<DRepr> --> IAbstractEventProcessor_DRepr
 **/
template <typename DRepr>
using IAbstractEventProcessor_ImplType = xo::facet::FacetImplType<AAbstractEventProcessor, DRepr>;

} /*namespace reactor*/
} /*namespace xo*/

/* AAbstractEventProcessor.hpp */
