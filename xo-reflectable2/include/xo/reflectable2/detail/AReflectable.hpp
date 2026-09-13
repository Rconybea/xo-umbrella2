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
#include <xo/reflect/TaggedPtr.hpp>
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
A representation opts in by implementing IReflectable_DRepr, whose
self_tp() hands back a TaggedPtr for itself.  FopTdx rotates an
erased obj<AFacet> here through FacetRegistry to recover it.
NOTE the TaggedPtr does not keep the object alive: fomo objects are
arena-allocated rather than refcounted, so validity belongs to the
owning flywheel.  Fine for synchronous traversal; a caller must not
retain one past the arena.
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
    /** reflect's (TypeDescr, address) pair **/
    using TaggedPtr = xo::reflect::TaggedPtr;
    ///@}

    /** @defgroup reflect-reflectable-methods **/
    ///@{
    // const methods
    /* _has_null_vptr(), _typeseq(), _drop(): inherited from xo::facet::ATop */

    // nonconst methods
    /** TaggedPtr for this object's concrete representation **/
    virtual TaggedPtr self_tp(Opaque data)  = 0;
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
