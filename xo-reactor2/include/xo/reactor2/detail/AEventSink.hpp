/** @file AEventSink.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/EventSink.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/EventSink.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/facet/ATop.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

namespace xo::reflect { class TypeDescrBase; }
namespace xo::reflect { using TypeDescr = TypeDescrBase const *; }
namespace xo::reflect { class TaggedPtr; }

namespace xo {
namespace reactor {

using Copaque = const void *;
using Opaque = void *;

/**
Trait for consuming a typed stream of events
**/
class AEventSink : public xo::facet::ATop {
public:
    /** @defgroup reactor-eventsink-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /** reflected type information **/
    using TypeDescr = xo::reflect::TypeDescr;
    /** variant object **/
    using TaggedPtr = xo::reflect::TaggedPtr;
    ///@}

    /** @defgroup reactor-eventsink-methods **/
    ///@{
    // const methods
    /* _has_null_vptr(), _typeseq(), _drop(): inherited from xo::facet::ATop */
    /** if true, accept events of any time at runtime (via tagged variant) **/
    virtual bool allow_polymorphic_source(Copaque data)  const  noexcept = 0;
    /** require incoming events satisfy this type (unless polymorphic source) **/
    virtual TypeDescr sink_ev_type(Copaque data)  const  noexcept = 0;
    /** if true, accept events that may be externally modified/destroyed after delivery **/
    virtual bool allow_volatile_source(Copaque data)  const  noexcept = 0;
    /** report lifetime number of incoming events at this sink **/
    virtual uint32_t n_in_ev(Copaque data)  const  noexcept = 0;

    // nonconst methods
    /** report incoming event to this sink **/
    virtual void notify_ev_tp(Opaque data, const TaggedPtr & ev_tp)  = 0;
    ///@}
}; /*AEventSink*/

/** Implementation IEventSink_DRepr of AEventSink for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AEventSink, DRepr> {
 *        using Impltype = IEventSink_DRepr;
 *    };
 *
 *  then IEventSink_ImplType<DRepr> --> IEventSink_DRepr
 **/
template <typename DRepr>
using IEventSink_ImplType = xo::facet::FacetImplType<AEventSink, DRepr>;

} /*namespace reactor*/
} /*namespace xo*/

/* AEventSink.hpp */
