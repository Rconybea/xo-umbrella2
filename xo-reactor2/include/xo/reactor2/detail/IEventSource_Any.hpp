/** @file IEventSource_Any.hpp
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
#include <xo/facet/obj.hpp>

namespace xo { namespace reactor { class IEventSource_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::reactor::AEventSource,
                           DVariantPlaceholder>
{
    using ImplType = xo::reactor::IEventSource_Any;
};

}
}

namespace xo {
namespace reactor {

    /** @class IEventSource_Any
     *  @brief AEventSource implementation for empty variant instance
     **/
    class IEventSource_Any : public AEventSource {
    public:
        /** @defgroup reactor-eventsource-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using CallbackId = AEventSource::CallbackId;
        using TypeDescr = AEventSource::TypeDescr;

        ///@}
        /** @defgroup reactor-eventsource-any-methods **/
        ///@{

        const AEventSource * iface() const { return std::launder(this); }

        // from AEventSource

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void _drop(Opaque) const noexcept override { _fatal(); }

        // const methods
        [[noreturn]] TypeDescr source_ev_type(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] bool is_volatile(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] uint32_t n_queued_out_ev(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] uint32_t n_out_ev(Copaque)  const  noexcept override { _fatal(); }

        // nonconst methods
        [[noreturn]] CallbackId attach_sink(Opaque, obj<AEventSink>)  override;
        [[noreturn]] void detach_sink(Opaque, CallbackId)  override;
        [[noreturn]] uint64_t deliver_one(Opaque)  override;

        ///@}

    private:
        /** @defgraoup reactor-eventsource-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup reactor-eventsource-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace reactor */
} /*namespace xo */

/* IEventSource_Any.hpp */
