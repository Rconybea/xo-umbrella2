/** @file RResourceVisitor.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ResourceVisitor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ResourceVisitor.json5]
 **/

#pragma once

#include "AResourceVisitor.hpp"

namespace xo {
namespace mm {

/** @class RResourceVisitor
 **/
template <typename Object>
class RResourceVisitor : public Object {
private:
    using O = Object;

public:
    /** @defgroup mm-resourcevisitor-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using size_type = AResourceVisitor::size_type;
    ///@}

    /** @defgroup mm-resourcevisitor-router-ctors **/
    ///@{
    RResourceVisitor() {}
    RResourceVisitor(Object::DataPtr data) : Object{std::move(data)} {}
    RResourceVisitor(const AResourceVisitor * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup mm-resourcevisitor-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    void on_allocator(obj<AAllocator> mm)  const  noexcept {
        return O::iface()->on_allocator(O::data(), mm);
    }

    // non-const methods (still const in router!)

    ///@}
    /** @defgroup mm-resourcevisitor-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RResourceVisitor<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace mm*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::mm::AResourceVisitor, Object> {
        using RoutingType = xo::mm::RResourceVisitor<Object>;
    };
} }

/* end RResourceVisitor.hpp */
