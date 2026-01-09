/** @file APrintable.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Printable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Printable.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

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
    /** dynamic pretty-printing state during layout **/
    using ppindentinfo = xo::print::ppindentinfo;
    ///@}

    /** @defgroup print-printable-methods **/
    ///@{
    // const methods
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** Pretty-printing support for this object.
        See [xo-indentlog/xo/indentlog/pretty.hpp]
    **/
    virtual bool pretty(Copaque data, const ppindentinfo & ppii) const = 0;

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

/*  */
