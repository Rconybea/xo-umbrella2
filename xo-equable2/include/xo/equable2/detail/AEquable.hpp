/** @file AEquable.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Equable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Equable.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/facet/ATop.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>


namespace xo {
namespace equable {

using Copaque = const void *;
using Opaque = void *;

/**
Trait for data types that support equality comparison.
PLACEHOLDER: the facet is not designed yet -- const_methods below is
deliberately empty, so AEquable presently has no methods.  The
subsystem is scaffolded and builds so that `xo-build --all` covers
every entry in xo-cmake/etc/xo/subsystem-list.
**/
class AEquable : public xo::facet::ATop {
public:
    /** @defgroup equable-equable-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /**  **/
    using obj_AEquable = xo::facet::obj<AEquable>;
    ///@}

    /** @defgroup equable-equable-methods **/
    ///@{
    // const methods
    /* _has_null_vptr(), _typeseq(), _drop(): inherited from xo::facet::ATop */

    // nonconst methods
    ///@}
}; /*AEquable*/

/** Implementation IEquable_DRepr of AEquable for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AEquable, DRepr> {
 *        using Impltype = IEquable_DRepr;
 *    };
 *
 *  then IEquable_ImplType<DRepr> --> IEquable_DRepr
 **/
template <typename DRepr>
using IEquable_ImplType = xo::facet::FacetImplType<AEquable, DRepr>;

} /*namespace equable*/
} /*namespace xo*/

/* AEquable.hpp */
