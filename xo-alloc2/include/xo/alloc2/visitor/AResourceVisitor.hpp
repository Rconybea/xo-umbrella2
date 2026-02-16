/** @file AResourceVisitor.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ResourceVisitor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ResourceVisitor.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include "Allocator.hpp"
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

// {pretext} here

namespace xo {
namespace mm {

using Copaque = const void *;
using Opaque = void *;

/**
Visitor to receive measured resource consumption
**/
class AResourceVisitor {
public:
    /** @defgroup mm-resourcevisitor-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /** type for length of a sequence **/
    using size_type = std::size_t;
    ///@}

    /** @defgroup mm-resourcevisitor-methods **/
    ///@{
    // const methods
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;
    /** report memory consumption **/
    virtual void on_allocator(Copaque data, obj<AAllocator> mm)  const  noexcept = 0;

    // nonconst methods
    ///@}
}; /*AResourceVisitor*/

/** Implementation IResourceVisitor_DRepr of AResourceVisitor for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AResourceVisitor, DRepr> {
 *        using Impltype = IResourceVisitor_DRepr;
 *    };
 *
 *  then IResourceVisitor_ImplType<DRepr> --> IResourceVisitor_DRepr
 **/
template <typename DRepr>
using IResourceVisitor_ImplType = xo::facet::FacetImplType<AResourceVisitor, DRepr>;

} /*namespace mm*/
} /*namespace xo*/

/* AResourceVisitor.hpp */
