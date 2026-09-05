/** @file AEventSource.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/EventSource.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/EventSource.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/facet/ATop.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

#include "EventSink.hpp"
#include <xo/callback/CallbackId.hpp>
#include <xo/reflect/TypeDescr.hpp>

namespace xo {
namespace reactor {

using Copaque = const void *;
using Opaque = void *;

/**
Trait for producing a typed stream of events
**/
class AEventSource : public xo::facet::ATop {
public:
    /** @defgroup reactor-eventsource-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /** identifies a source->sink connection **/
    using CallbackId = xo::fn::CallbackId;
    /** reflected type information **/
    using TypeDescr = xo::reflect::TypeDescr;
    ///@}

    /** @defgroup reactor-eventsource-methods **/
    ///@{
    // const methods
    /* _has_null_vptr(), _typeseq(), _drop(): inherited from xo::facet::ATop */
    /** promise produced events satisfy this type **/
    virtual TypeDescr source_ev_type(Copaque data)  const  noexcept = 0;
    /** true if event objects may be overwritten between callbacks. **/
    virtual bool is_volatile(Copaque data)  const  noexcept = 0;
    /** counts outbound events ready for delivery, but not yet sent **/
    virtual uint32_t n_queued_out_ev(Copaque data)  const  noexcept = 0;
    /** lifetime number of events produced by this source **/
    virtual uint32_t n_out_ev(Copaque data)  const  noexcept = 0;

    // nonconst methods
    /** attach sink to this source. Return value can be used to detach later **/
    virtual CallbackId attach_sink(Opaque data, obj<AEventSink> sink)  = 0;
    /** detach sink from this source **/
    virtual void detach_sink(Opaque data, CallbackId id)  = 0;
    /** force delivery of one event. Returns number of events delivered. Prefer reactor **/
    virtual uint64_t deliver_one(Opaque data)  = 0;
    ///@}
}; /*AEventSource*/

/** Implementation IEventSource_DRepr of AEventSource for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AEventSource, DRepr> {
 *        using Impltype = IEventSource_DRepr;
 *    };
 *
 *  then IEventSource_ImplType<DRepr> --> IEventSource_DRepr
 **/
template <typename DRepr>
using IEventSource_ImplType = xo::facet::FacetImplType<AEventSource, DRepr>;

} /*namespace reactor*/
} /*namespace xo*/

/* AEventSource.hpp */
