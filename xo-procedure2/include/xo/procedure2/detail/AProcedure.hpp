/** @file AProcedure.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Procedure.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Procedure.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include "RuntimeContext.hpp"
#include <xo/gc/GCObject.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

namespace xo { namespace scm { class DArray; } }

namespace xo {
namespace scm {

using Copaque = const void *;
using Opaque = void *;

/**
Abstraction for a schematika procedure
**/
class AProcedure {
public:
    /** @defgroup scm-procedure-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /** a gc-aware object **/
    using AGCObject = xo::mm::AGCObject;
    ///@}

    /** @defgroup scm-procedure-methods **/
    ///@{
    // const methods
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;
    /** true iff procedure takes n arguments **/
    virtual bool is_nary(Copaque data)  const  noexcept = 0;
    /** number of arguments. -1 for n-ary **/
    virtual std::int32_t n_args(Copaque data)  const  noexcept = 0;

    // nonconst methods
    /** invoke procedure; assume arguments satisfy type system **/
    virtual obj<AGCObject> apply_nocheck(Opaque data, obj<ARuntimeContext> rcx, const DArray * args)  = 0;
    ///@}
}; /*AProcedure*/

/** Implementation IProcedure_DRepr of AProcedure for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AProcedure, DRepr> {
 *        using Impltype = IProcedure_DRepr;
 *    };
 *
 *  then IProcedure_ImplType<DRepr> --> IProcedure_DRepr
 **/
template <typename DRepr>
using IProcedure_ImplType = xo::facet::FacetImplType<AProcedure, DRepr>;

} /*namespace scm*/
} /*namespace xo*/

/* AProcedure.hpp */
