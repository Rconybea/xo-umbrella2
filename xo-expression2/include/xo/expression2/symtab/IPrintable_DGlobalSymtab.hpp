/** @file IPrintable_DGlobalSymtab.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DGlobalSymtab.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DGlobalSymtab.json5]
 **/

#pragma once

#include "Printable.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>
#include "DGlobalSymtab.hpp"

namespace xo { namespace scm { class IPrintable_DGlobalSymtab; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::print::APrintable,
                                   xo::scm::DGlobalSymtab>
        {
            using ImplType = xo::print::IPrintable_Xfer
              <xo::scm::DGlobalSymtab,
               xo::scm::IPrintable_DGlobalSymtab>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IPrintable_DGlobalSymtab
         **/
        class IPrintable_DGlobalSymtab {
        public:
            /** @defgroup scm-printable-dglobalsymtab-type-traits **/
            ///@{
            using PpSink = xo::print::APrintable::PpSink;
            using Copaque = xo::print::APrintable::Copaque;
            using Opaque = xo::print::APrintable::Opaque;
            ///@}
            /** @defgroup scm-printable-dglobalsymtab-methods **/
            ///@{
            // const methods
            /** Pretty-printing support for this object.
Single-pass: render into @p sink.  A nested object
participates in the enclosing structure's line breaking,
where the deprecated two-pass protocol could not.
See [xo-ppsink/xo/ppsink/PpSink.hpp] **/
            static void pretty(const DGlobalSymtab & self, PpSink & sink);

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */