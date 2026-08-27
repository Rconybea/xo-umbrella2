/** @file IEventSource_Xfer.hpp
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
 *
 *  variables:
 *    {facet_hpp_fname} -> EventSource.hpp
 *    {impl_hpp_subdir} -> detail
 *    {facet_ns1} -> xo
 *    {facet_detail_subdir} -> detail
 *    {abstract_facet_fname} -> AEventSource.hpp
 **/

#pragma once

#include "AEventSource.hpp"

namespace xo {
namespace reactor {
    /** @class IEventSource_Xfer
     **/
    template <typename DRepr, typename IEventSource_DRepr>
    class IEventSource_Xfer : public AEventSource {
    public:
        /** @defgroup reactor-eventsource-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IEventSource_DRepr;
        /** integer identifying a type **/
        using typeseq = AEventSource::typeseq;
        using CallbackId = AEventSource::CallbackId;
        using TypeDescr = AEventSource::TypeDescr;
        ///@}

        /** @defgroup reactor-eventsource-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AEventSource

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        TypeDescr source_ev_type(Copaque data)  const  noexcept override {
            return I::source_ev_type(_dcast(data));
        }
        bool is_volatile(Copaque data)  const  noexcept override {
            return I::is_volatile(_dcast(data));
        }
        uint32_t n_queued_out_ev(Copaque data)  const  noexcept override {
            return I::n_queued_out_ev(_dcast(data));
        }
        uint32_t n_out_ev(Copaque data)  const  noexcept override {
            return I::n_out_ev(_dcast(data));
        }

        // non-const methods
        CallbackId attach_sink(Opaque data, obj<AEventSink> sink)  override {
            return I::attach_sink(_dcast(data), sink);
        }
        void detach_sink(Opaque data, CallbackId id)  override {
            return I::detach_sink(_dcast(data), id);
        }
        uint64_t deliver_one(Opaque data)  override {
            return I::deliver_one(_dcast(data));
        }

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup reactor-eventsource-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IEventSource_DRepr>
    xo::facet::typeseq
    IEventSource_Xfer<DRepr, IEventSource_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IEventSource_DRepr>
    bool
    IEventSource_Xfer<DRepr, IEventSource_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AEventSource,
                                              IEventSource_Xfer>();

} /*namespace reactor */
} /*namespace xo*/

/* end IEventSource_Xfer.hpp */
