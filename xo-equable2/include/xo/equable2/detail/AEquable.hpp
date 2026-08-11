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
class AEquable {
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
    /** An uninitialized AEquable instance will have zero vtable pointer (per {linux,osx} abi).
     *  Use case for this is narrow. We go to some lengths to avoid null vtable pointers. For example
     *  obj<AFacet> will have non-null vtable (via IFacet_Any) with all methods terminating.
     **/
    bool _has_null_vptr() const noexcept { return *reinterpret_cast<const void * const *>(this) == nullptr; }
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;

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
