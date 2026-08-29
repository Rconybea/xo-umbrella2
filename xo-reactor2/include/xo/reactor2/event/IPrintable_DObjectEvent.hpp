/** @file IPrintable_DObjectEvent.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DObjectEvent.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DObjectEvent.json5]
 **/

#pragma once

#include "Printable.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>
#include "DObjectEvent.hpp"

namespace xo { namespace process { class IPrintable_DObjectEvent; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::print::APrintable,
                                   xo::process::DObjectEvent>
        {
            using ImplType = xo::print::IPrintable_Xfer
              <xo::process::DObjectEvent,
               xo::process::IPrintable_DObjectEvent>;
        };
    }
}

namespace xo {
    namespace process {
        /** @class IPrintable_DObjectEvent
         **/
        class IPrintable_DObjectEvent {
        public:
            /** @defgroup process-printable-dobjectevent-type-traits **/
            ///@{
            using PpSink = xo::print::APrintable::PpSink;
            using Copaque = xo::print::APrintable::Copaque;
            using Opaque = xo::print::APrintable::Opaque;
            ///@}
            /** @defgroup process-printable-dobjectevent-methods **/
            ///@{
            // const methods
            /** Pretty-printing support for this object.
Single-pass: render into @p sink.  A nested object
participates in the enclosing structure's line breaking,
where the deprecated two-pass protocol could not.
See [xo-ppsink/xo/ppsink/PpSink.hpp] **/
            static void pretty(const DObjectEvent & self, PpSink & sink);

            // non-const methods
            ///@}
        };

    } /*namespace process*/
} /*namespace xo*/

/* end */