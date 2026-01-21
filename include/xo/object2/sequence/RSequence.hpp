/** @file RSequence.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2-claude1/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Sequence.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Sequence.json5]
 **/

#pragma once

#include "ASequence.hpp"

namespace xo {
namespace scm {

/** @class RSequence
 **/
template <typename Object>
class RSequence : public Object {
private:
    using O = Object;

public:
    /** @defgroup scm-sequence-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using size_type = ASequence::size_type;
    using AGCObject = ASequence::AGCObject;
    ///@}

    /** @defgroup scm-sequence-router-ctors **/
    ///@{
    RSequence() {}
    RSequence(Object::DataPtr data) : Object{std::move(data)} {}
    RSequence(const ASequence * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup scm-sequence-router-methods **/
    ///@{

    // const methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    bool is_empty()  const  noexcept {
        return O::iface()->is_empty(O::data());
    }
    bool is_finite()  const  noexcept {
        return O::iface()->is_finite(O::data());
    }
    obj<AGCObject> at(size_type index)  const {
        return O::iface()->at(O::data(), index);
    }

    // non-const methods (still const in router!)

    ///@}
    /** @defgroup scm-sequence-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RSequence<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace scm*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::scm::ASequence, Object> {
        using RoutingType = xo::scm::RSequence<Object>;
    };
} }

/* end RSequence.hpp */