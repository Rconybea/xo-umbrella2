/** @file RProcedure.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Procedure.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Procedure.json5]
 **/

#pragma once

#include "AProcedure.hpp"

namespace xo {
namespace scm {

/** @class RProcedure
 **/
template <typename Object>
class RProcedure : public Object {
private:
    using O = Object;

public:
    /** @defgroup scm-procedure-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using AGCObject = AProcedure::AGCObject;
    ///@}

    /** @defgroup scm-procedure-router-ctors **/
    ///@{
    RProcedure() {}
    RProcedure(Object::DataPtr data) : Object{std::move(data)} {}
    RProcedure(const AProcedure * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup scm-procedure-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    bool is_nary()  const  noexcept {
        return O::iface()->is_nary(O::data());
    }
    std::int32_t n_args()  const  noexcept {
        return O::iface()->n_args(O::data());
    }

    // non-const methods (still const in router!)
    obj<AGCObject> apply_nocheck(obj<ARuntimeContext> rcx, const DArray * args)  {
        return O::iface()->apply_nocheck(O::data(), rcx, args);
    }

    ///@}
    /** @defgroup scm-procedure-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RProcedure<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace scm*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::scm::AProcedure, Object> {
        using RoutingType = xo::scm::RProcedure<Object>;
    };
} }

/* end RProcedure.hpp */
