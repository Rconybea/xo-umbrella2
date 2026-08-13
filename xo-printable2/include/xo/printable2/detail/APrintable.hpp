/** @file APrintable.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Printable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Printable.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/ppsink/PpSink.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

// {pretext} here

namespace xo {
namespace print {

using Copaque = const void *;
using Opaque = void *;

/**
Trait for data types that support pretty-printing
**/
class APrintable {
public:
    /** @defgroup print-printable-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /** single-pass pretty-printing sink; see xo-ppsink **/
    using PpSink = xo::pp::PpSink;
    ///@}

    /** @defgroup print-printable-methods **/
    ///@{
    // const methods
    /** An uninitialized APrintable instance will have zero vtable pointer (per {linux,osx} abi).
     *  Use case for this is narrow. We go to some lengths to avoid null vtable pointers. For example
     *  obj<AFacet> will have non-null vtable (via IFacet_Any) with all methods terminating.
     **/
    bool _has_null_vptr() const noexcept { return *reinterpret_cast<const void * const *>(this) == nullptr; }
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;
    /** Pretty-printing support for this object.
Single-pass: render into @p sink.  A nested object
participates in the enclosing structure's line breaking,
where the deprecated two-pass protocol could not.
See [xo-ppsink/xo/ppsink/PpSink.hpp] **/
    virtual void pretty(Copaque data, PpSink & sink)  const = 0;

    // nonconst methods
    ///@}
}; /*APrintable*/

/** Implementation IPrintable_DRepr of APrintable for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<APrintable, DRepr> {
 *        using Impltype = IPrintable_DRepr;
 *    };
 *
 *  then IPrintable_ImplType<DRepr> --> IPrintable_DRepr
 **/
template <typename DRepr>
using IPrintable_ImplType = xo::facet::FacetImplType<APrintable, DRepr>;

} /*namespace print*/
} /*namespace xo*/

/* APrintable.hpp */
