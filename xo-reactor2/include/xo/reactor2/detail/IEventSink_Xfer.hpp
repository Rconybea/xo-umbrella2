/** @file IEventSink_Xfer.hpp
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
 *
 *  variables:
 *    {facet_hpp_fname} -> EventSink.hpp
 *    {impl_hpp_subdir} -> detail
 *    {facet_ns1} -> xo
 *    {facet_detail_subdir} -> detail
 *    {abstract_facet_fname} -> AEventSink.hpp
 **/

#pragma once

#include "AEventSink.hpp"

namespace xo {
namespace reactor {
    /** @class IEventSink_Xfer
     **/
    template <typename DRepr, typename IEventSink_DRepr>
    class IEventSink_Xfer : public AEventSink {
    public:
        /** @defgroup reactor-eventsink-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IEventSink_DRepr;
        /** integer identifying a type **/
        using typeseq = AEventSink::typeseq;
        using TypeDescr = AEventSink::TypeDescr;
        using TaggedPtr = AEventSink::TaggedPtr;
        ///@}

        /** @defgroup reactor-eventsink-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AEventSink

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        bool allow_polymorphic_source(Copaque data)  const  noexcept override {
            return I::allow_polymorphic_source(_dcast(data));
        }
        TypeDescr sink_ev_type(Copaque data)  const  noexcept override {
            return I::sink_ev_type(_dcast(data));
        }
        bool allow_volatile_source(Copaque data)  const  noexcept override {
            return I::allow_volatile_source(_dcast(data));
        }
        uint32_t n_in_ev(Copaque data)  const  noexcept override {
            return I::n_in_ev(_dcast(data));
        }

        // non-const methods
        void notify_ev_tp(Opaque data, const TaggedPtr & ev_tp)  override {
            return I::notify_ev_tp(_dcast(data), ev_tp);
        }

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup reactor-eventsink-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IEventSink_DRepr>
    xo::facet::typeseq
    IEventSink_Xfer<DRepr, IEventSink_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IEventSink_DRepr>
    bool
    IEventSink_Xfer<DRepr, IEventSink_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AEventSink,
                                              IEventSink_Xfer>();

} /*namespace reactor */
} /*namespace xo*/

/* end IEventSink_Xfer.hpp */
