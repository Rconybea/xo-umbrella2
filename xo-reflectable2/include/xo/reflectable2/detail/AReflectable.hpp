/** @file AReflectable.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Reflectable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Reflectable.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/facet/top/ATop.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>


namespace xo {
namespace reflect {

using Copaque = const void *;
using Opaque = void *;

/**
Opt-in capability for faceted objects that can be interrogated at runtime.
Counterpart to xo::reflect::SelfTagging, which solves the same
get-me-the-real-type-at-runtime problem for non-fomo objects.
SCAFFOLD: const_methods below is deliberately empty, so AReflectable
presently has no methods.  The method -- self_tp(), returning a
TaggedPtr for the object's concrete representation -- arrives with
the rotation through FacetRegistry.  See .xo-backlog/reflectable2/.
**/
class AReflectable : public xo::facet::ATop {
public:
    /** @defgroup reflect-reflectable-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /**  **/
    using obj_AReflectable = xo::facet::obj<AReflectable>;
    ///@}

    /** @defgroup reflect-reflectable-methods **/
    ///@{
    // const methods
    /* _has_null_vptr(), _typeseq(), _drop(): inherited from xo::facet::ATop */

    // nonconst methods
    ///@}
}; /*AReflectable*/

/** Implementation IReflectable_DRepr of AReflectable for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AReflectable, DRepr> {
 *        using Impltype = IReflectable_DRepr;
 *    };
 *
 *  then IReflectable_ImplType<DRepr> --> IReflectable_DRepr
 **/
template <typename DRepr>
using IReflectable_ImplType = xo::facet::FacetImplType<AReflectable, DRepr>;

} /*namespace reflect*/
} /*namespace xo*/

/* AReflectable.hpp */
