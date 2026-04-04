/** @file AType.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Type.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Type.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/type/Metatype.hpp>
#include <xo/reflect/TypeDescr.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

// pretext if any

namespace xo {
namespace scm {

using Copaque = const void *;
using Opaque = void *;

/**
1. Ability to compare types as members of partial order
2. ...
**/
class AType {
public:
    /** @defgroup scm-type-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /**  **/
    using obj_AType = xo::facet::obj<AType>;
    /**  **/
    using TypeDescr = xo::reflect::TypeDescr;
    ///@}

    /** @defgroup scm-type-methods **/
    ///@{
    // const methods
    /** An uninitialized AType instance will have zero vtable pointer (per {linux,osx} abi).
     *  Use case for this is narrow. We go to some lengths to avoid null vtable pointers. For example
     *  obj<AFacet> will have non-null vtable (via IFacet_Any) with all methods terminating.
     **/
    bool _has_null_vptr() const noexcept { return *reinterpret_cast<const void * const *>(this) == nullptr; }
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;
    /** category for this type **/
    virtual Metatype metatype(Copaque data)  const  noexcept = 0;
    /** reflected representation for instances of this type **/
    virtual TypeDescr repr_td(Copaque data)  const  noexcept = 0;
    /** true iff this type is equal to y **/
    virtual bool is_equal_to(Copaque data, const obj_AType & y)  const = 0;
    /** true iff this is a subtype of y **/
    virtual bool is_subtype_of(Copaque data, const obj_AType & y)  const = 0;

    // nonconst methods
    ///@}
}; /*AType*/

/** Implementation IType_DRepr of AType for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AType, DRepr> {
 *        using Impltype = IType_DRepr;
 *    };
 *
 *  then IType_ImplType<DRepr> --> IType_DRepr
 **/
template <typename DRepr>
using IType_ImplType = xo::facet::FacetImplType<AType, DRepr>;

} /*namespace scm*/
} /*namespace xo*/

/* AType.hpp */
