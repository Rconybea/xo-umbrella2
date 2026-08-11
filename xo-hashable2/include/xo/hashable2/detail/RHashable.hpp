/** @file RHashable.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Hashable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Hashable.json5]
 **/

#pragma once

#include "AHashable.hpp"

namespace xo {
namespace hashable {

/** @class RHashable
 **/
template <typename Object>
class RHashable : public Object {
private:
    using O = Object;

public:
    /** @defgroup hashable-hashable-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using obj_AHashable = AHashable::obj_AHashable;
    ///@}

    /** @defgroup hashable-hashable-router-ctors **/
    ///@{
    RHashable() {}
    RHashable(Object::DataPtr data) : Object{std::move(data)} {}
    RHashable(const AHashable * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup hashable-hashable-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods

    // non-const methods (still const in router!)

    ///@}
    /** @defgroup hashable-hashable-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RHashable<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace hashable*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::hashable::AHashable, Object> {
        using RoutingType = xo::hashable::RHashable<Object>;
    };
} }

/* end RHashable.hpp */
