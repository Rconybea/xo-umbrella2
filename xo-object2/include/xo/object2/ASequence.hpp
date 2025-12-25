/** @file ASequence.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-object2/../xo-facet/codegen/genfacet.py]
 *     arguments:
 *       --input [./idl/Sequence.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [./idl/Sequence.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/gc/GCObject.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

namespace xo {
namespace scm {

using Copaque = const void *;
using Opaque = void *;

/**
Elements appear in some determinstic order.
Sequence is GC-aware --> elements must be GC-aware
**/
class ASequence {
public:
    /** @defgroup scm-sequence-type-traits **/
    ///@{
    // types
    /** type for length of a sequence **/
    using size_type = std::size_t;
    /** facet for types with GC support **/
    using AGCObject = xo::mm::AGCObject;
    ///@}

    /** @defgroup scm-sequence-methods **/
    ///@{
    // const methods
    /** RTTI: unique id# for actual runtime data representation **/
    virtual int32_t _typeseq() const noexcept = 0;
    /** true iff sequence is empty **/
    virtual bool is_empty(Copaque data) const noexcept = 0;
    /** true iff sequence is finite **/
    virtual bool is_finite(Copaque data) const noexcept = 0;
    /** return element @p index of this sequence **/
    virtual obj<AGCObject> at(Copaque data, size_type index) const = 0;

    // nonconst methods
    ///@}
}; /*ASequence*/

/** Implementation ISequence_DRepr of ASequence for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<ASequence, DRepr> {
 *        using Impltype = ISequence_DRepr;
 *    };
 *
 *  then ISequence_ImplType<DRepr> --> ISequence_DRepr
 **/
template <typename DRepr>
using ISequence_ImplType = xo::facet::FacetImplType<ASequence, DRepr>;

} /*namespace scm*/
} /*namespace xo*/

/*  */