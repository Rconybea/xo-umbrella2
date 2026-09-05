/** @file REventSource.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/EventSource.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/EventSource.json5]
 **/

#pragma once

#include "AEventSource.hpp"

namespace xo {
namespace reactor {

/** @class REventSource
 **/
template <typename Object>
class REventSource : public Object {
private:
    using O = Object;

public:
    /** @defgroup reactor-eventsource-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using CallbackId = AEventSource::CallbackId;
    using TypeDescr = AEventSource::TypeDescr;
    ///@}

    /** @defgroup reactor-eventsource-router-ctors **/
    ///@{
    REventSource() {}
    REventSource(Object::DataPtr data) : Object{std::move(data)} {}
    REventSource(const AEventSource * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup reactor-eventsource-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    bool _has_null_vptr() const noexcept { return O::iface()->_has_null_vptr(); }
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    TypeDescr source_ev_type()  const  noexcept {
        return O::iface()->source_ev_type(O::data());
    }
    bool is_volatile()  const  noexcept {
        return O::iface()->is_volatile(O::data());
    }
    uint32_t n_queued_out_ev()  const  noexcept {
        return O::iface()->n_queued_out_ev(O::data());
    }
    uint32_t n_out_ev()  const  noexcept {
        return O::iface()->n_out_ev(O::data());
    }

    // non-const methods (still const in router!)
    CallbackId attach_sink(obj<AEventSink> sink)  {
        return O::iface()->attach_sink(O::data(), sink);
    }
    void detach_sink(CallbackId id)  {
        return O::iface()->detach_sink(O::data(), id);
    }
    uint64_t deliver_one()  {
        return O::iface()->deliver_one(O::data());
    }

    ///@}
    /** @defgroup reactor-eventsource-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
REventSource<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace reactor*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::reactor::AEventSource, Object> {
        using RoutingType = xo::reactor::REventSource<Object>;
    };
} }

/* end REventSource.hpp */
