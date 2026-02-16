/** @file RRuntimeContext.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/RuntimeContext.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/RuntimeContext.json5]
 **/

#pragma once

#include "ARuntimeContext.hpp"

namespace xo {
namespace scm {

/** @class RRuntimeContext
 **/
template <typename Object>
class RRuntimeContext : public Object {
private:
    using O = Object;

public:
    /** @defgroup scm-runtimecontext-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using AAllocator = ARuntimeContext::AAllocator;
    ///@}

    /** @defgroup scm-runtimecontext-router-ctors **/
    ///@{
    RRuntimeContext() {}
    RRuntimeContext(Object::DataPtr data) : Object{std::move(data)} {}
    RRuntimeContext(const ARuntimeContext * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup scm-runtimecontext-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    obj<AAllocator> allocator()  const  noexcept {
        return O::iface()->allocator(O::data());
    }

    // non-const methods (still const in router!)

    ///@}
    /** @defgroup scm-runtimecontext-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RRuntimeContext<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace scm*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::scm::ARuntimeContext, Object> {
        using RoutingType = xo::scm::RRuntimeContext<Object>;
    };
} }

/* end RRuntimeContext.hpp */
