/** @file RReflectable.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Reflectable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Reflectable.json5]
 **/

#pragma once

#include "AReflectable.hpp"

namespace xo {
namespace reflect {

/** @class RReflectable
 **/
template <typename Object>
class RReflectable : public Object {
private:
    using O = Object;

public:
    /** @defgroup reflect-reflectable-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using obj_AReflectable = AReflectable::obj_AReflectable;
    ///@}

    /** @defgroup reflect-reflectable-router-ctors **/
    ///@{
    RReflectable() {}
    RReflectable(Object::DataPtr data) : Object{std::move(data)} {}
    RReflectable(const AReflectable * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup reflect-reflectable-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    bool _has_null_vptr() const noexcept { return O::iface()->_has_null_vptr(); }
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods

    // non-const methods (still const in router!)

    ///@}
    /** @defgroup reflect-reflectable-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RReflectable<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace reflect*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::reflect::AReflectable, Object> {
        using RoutingType = xo::reflect::RReflectable<Object>;
    };
} }

/* end RReflectable.hpp */
