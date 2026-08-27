/** @file IEventSink_Any.hpp
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
#include <xo/facet/obj.hpp>

namespace xo { namespace reactor { class IEventSink_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::reactor::AEventSink,
                           DVariantPlaceholder>
{
    using ImplType = xo::reactor::IEventSink_Any;
};

}
}

namespace xo {
namespace reactor {

    /** @class IEventSink_Any
     *  @brief AEventSink implementation for empty variant instance
     **/
    class IEventSink_Any : public AEventSink {
    public:
        /** @defgroup reactor-eventsink-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using TypeDescr = AEventSink::TypeDescr;
        using TaggedPtr = AEventSink::TaggedPtr;

        ///@}
        /** @defgroup reactor-eventsink-any-methods **/
        ///@{

        const AEventSink * iface() const { return std::launder(this); }

        // from AEventSink

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void _drop(Opaque) const noexcept override { _fatal(); }

        // const methods
        [[noreturn]] bool allow_polymorphic_source(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] TypeDescr sink_ev_type(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] bool allow_volatile_source(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] uint32_t n_in_ev(Copaque)  const  noexcept override { _fatal(); }

        // nonconst methods
        [[noreturn]] void notify_ev_tp(Opaque, const TaggedPtr &)  override;

        ///@}

    private:
        /** @defgraoup reactor-eventsink-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup reactor-eventsink-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace reactor */
} /*namespace xo */

/* IEventSink_Any.hpp */
