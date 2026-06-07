/** @file RType.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Type.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Type.json5]
 **/

#pragma once

#include "AType.hpp"

namespace xo {
namespace scm {

/** @class RType
 **/
template <typename Object>
class RType : public Object {
private:
    using O = Object;

public:
    /** @defgroup scm-type-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using obj_AType = AType::obj_AType;
    using TypeDescr = AType::TypeDescr;
    ///@}

    /** @defgroup scm-type-router-ctors **/
    ///@{
    RType() {}
    RType(Object::DataPtr data) : Object{std::move(data)} {}
    RType(const AType * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup scm-type-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    Metatype metatype()  const  noexcept {
        return O::iface()->metatype(O::data());
    }
    TypeDescr repr_td()  const  noexcept {
        return O::iface()->repr_td(O::data());
    }
    bool is_equal_to(const obj_AType & y)  const {
        return O::iface()->is_equal_to(O::data(), y);
    }
    bool is_subtype_of(const obj_AType & y)  const {
        return O::iface()->is_subtype_of(O::data(), y);
    }

    // non-const methods (still const in router!)

    ///@}
    /** @defgroup scm-type-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RType<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace scm*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::scm::AType, Object> {
        using RoutingType = xo::scm::RType<Object>;
    };
} }

/* end RType.hpp */
