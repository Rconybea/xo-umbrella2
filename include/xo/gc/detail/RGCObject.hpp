/** @file RGCObject.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/GCObject.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/GCObject.json5]
 **/

#pragma once

#include "AGCObject.hpp"

namespace xo {
namespace mm {

/** @class RGCObject
 **/
template <typename Object>
class RGCObject : public Object {
private:
    using O = Object;

public:
    /** @defgroup mm-gcobject-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using size_type = AGCObject::size_type;
    using AAllocator = AGCObject::AAllocator;
    using ACollector = AGCObject::ACollector;
    ///@}

    /** @defgroup mm-gcobject-router-ctors **/
    ///@{
    RGCObject() {}
    RGCObject(Object::DataPtr data) : Object{std::move(data)} {}
    RGCObject(const AGCObject * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup mm-gcobject-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    size_type shallow_size()  const  noexcept {
        return O::iface()->shallow_size(O::data());
    }
    Opaque shallow_copy(obj<AAllocator> mm)  const  noexcept {
        return O::iface()->shallow_copy(O::data(), mm);
    }

    // non-const methods (still const in router!)
    size_type forward_children(obj<ACollector> gc)  noexcept {
        return O::iface()->forward_children(O::data(), gc);
    }

    ///@}
    /** @defgroup mm-gcobject-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RGCObject<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace mm*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::mm::AGCObject, Object> {
        using RoutingType = xo::mm::RGCObject<Object>;
    };
} }

/* end RGCObject.hpp */
