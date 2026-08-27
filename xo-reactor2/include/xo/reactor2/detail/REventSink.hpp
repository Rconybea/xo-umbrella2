/** @file REventSink.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/EventSink.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/EventSink.json5]
 **/

#pragma once

#include "AEventSink.hpp"

namespace xo {
namespace reactor {

/** @class REventSink
 **/
template <typename Object>
class REventSink : public Object {
private:
    using O = Object;

public:
    /** @defgroup reactor-eventsink-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using TypeDescr = AEventSink::TypeDescr;
    using TaggedPtr = AEventSink::TaggedPtr;
    ///@}

    /** @defgroup reactor-eventsink-router-ctors **/
    ///@{
    REventSink() {}
    REventSink(Object::DataPtr data) : Object{std::move(data)} {}
    REventSink(const AEventSink * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup reactor-eventsink-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    bool allow_polymorphic_source()  const  noexcept {
        return O::iface()->allow_polymorphic_source(O::data());
    }
    TypeDescr sink_ev_type()  const  noexcept {
        return O::iface()->sink_ev_type(O::data());
    }
    bool allow_volatile_source()  const  noexcept {
        return O::iface()->allow_volatile_source(O::data());
    }
    uint32_t n_in_ev()  const  noexcept {
        return O::iface()->n_in_ev(O::data());
    }

    // non-const methods (still const in router!)
    void notify_ev_tp(const TaggedPtr & ev_tp)  {
        return O::iface()->notify_ev_tp(O::data(), ev_tp);
    }

    ///@}
    /** @defgroup reactor-eventsink-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
REventSink<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace reactor*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::reactor::AEventSink, Object> {
        using RoutingType = xo::reactor::REventSink<Object>;
    };
} }

/* end REventSink.hpp */
