/** @file AHashable.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Hashable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Hashable.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>


namespace xo {
namespace hashable {

using Copaque = const void *;
using Opaque = void *;

/**
Trait for data types that can supply a hash of their contents.
PLACEHOLDER: the facet is not designed yet -- const_methods below is
deliberately empty, so AHashable presently has no methods.  The
subsystem is scaffolded and builds so that `xo-build --all` covers
every entry in xo-cmake/etc/xo/subsystem-list.
**/
class AHashable {
public:
    /** @defgroup hashable-hashable-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /**  **/
    using obj_AHashable = xo::facet::obj<AHashable>;
    ///@}

    /** @defgroup hashable-hashable-methods **/
    ///@{
    // const methods
    /** An uninitialized AHashable instance will have zero vtable pointer (per {linux,osx} abi).
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
}; /*AHashable*/

/** Implementation IHashable_DRepr of AHashable for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AHashable, DRepr> {
 *        using Impltype = IHashable_DRepr;
 *    };
 *
 *  then IHashable_ImplType<DRepr> --> IHashable_DRepr
 **/
template <typename DRepr>
using IHashable_ImplType = xo::facet::FacetImplType<AHashable, DRepr>;

} /*namespace hashable*/
} /*namespace xo*/

/* AHashable.hpp */
