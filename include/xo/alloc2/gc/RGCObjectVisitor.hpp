/** @file RGCObjectVisitor.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/GCObjectVisitor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/GCObjectVisitor.json5]
 **/

#pragma once

#include "AGCObjectVisitor.hpp"

namespace xo {
namespace mm {

/** @class RGCObjectVisitor
 **/
template <typename Object>
class RGCObjectVisitor : public Object {
private:
    using O = Object;

public:
    /** @defgroup mm-gcobjectvisitor-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    ///@}

    /** @defgroup mm-gcobjectvisitor-router-ctors **/
    ///@{
    RGCObjectVisitor() {}
    RGCObjectVisitor(Object::DataPtr data) : Object{std::move(data)} {}
    RGCObjectVisitor(const AGCObjectVisitor * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup mm-gcobjectvisitor-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods

    // non-const methods (still const in router!)
    void visit_child(AGCObject * iface, void ** pp_data)  noexcept {
        return O::iface()->visit_child(O::data(), iface, pp_data);
    }

    ///@}
    /** @defgroup mm-gcobjectvisitor-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RGCObjectVisitor<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace mm*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::mm::AGCObjectVisitor, Object> {
        using RoutingType = xo::mm::RGCObjectVisitor<Object>;
    };
} }

/* end RGCObjectVisitor.hpp */
