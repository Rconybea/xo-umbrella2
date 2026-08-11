/** @file REquable.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Equable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Equable.json5]
 **/

#pragma once

#include "AEquable.hpp"

namespace xo {
namespace equable {

/** @class REquable
 **/
template <typename Object>
class REquable : public Object {
private:
    using O = Object;

public:
    /** @defgroup equable-equable-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using obj_AEquable = AEquable::obj_AEquable;
    ///@}

    /** @defgroup equable-equable-router-ctors **/
    ///@{
    REquable() {}
    REquable(Object::DataPtr data) : Object{std::move(data)} {}
    REquable(const AEquable * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup equable-equable-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods

    // non-const methods (still const in router!)

    ///@}
    /** @defgroup equable-equable-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
REquable<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace equable*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::equable::AEquable, Object> {
        using RoutingType = xo::equable::REquable<Object>;
    };
} }

/* end REquable.hpp */
