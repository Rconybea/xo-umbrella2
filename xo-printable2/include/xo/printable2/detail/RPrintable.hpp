/** @file RPrintable.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Printable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Printable.json5]
 **/

#pragma once

#include "APrintable.hpp"

namespace xo {
namespace print {

/** @class RPrintable
 **/
template <typename Object>
class RPrintable : public Object {
private:
    using O = Object;

public:
    /** @defgroup print-printable-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using ppindentinfo = APrintable::ppindentinfo;
    ///@}

    /** @defgroup print-printable-router-ctors **/
    ///@{
    RPrintable() {}
    RPrintable(Object::DataPtr data) : Object{std::move(data)} {}
    RPrintable(const APrintable * iface, void * data)
        requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
    : Object(iface, data) {}

    ///@}
    /** @defgroup print-printable-router-methods **/
    ///@{

    // const methods
    int32_t _typeseq() const noexcept { return O::iface()->_typeseq(); }
    bool pretty(const ppindentinfo & ppii) {
        return O::iface()->pretty(O::data(), ppii);
    }

    // non-const methods
    // << do something for non-const methods >>
    //

    ///@}
    /** @defgroup print-printable-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RPrintable<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace print*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::print::APrintable, Object> {
        using RoutingType = xo::print::RPrintable<Object>;
    };
} }

/* end RPrintable.hpp */
