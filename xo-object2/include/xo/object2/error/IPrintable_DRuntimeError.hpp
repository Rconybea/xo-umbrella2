/** @file IPrintable_DRuntimeError.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DRuntimeError.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DRuntimeError.json5]
 **/

#pragma once

#include "DRuntimeError.hpp"
#include "Printable.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>

namespace xo { namespace scm { class IPrintable_DRuntimeError; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::print::APrintable,
                                   xo::scm::DRuntimeError>
        {
            using ImplType = xo::print::IPrintable_Xfer
              <xo::scm::DRuntimeError,
               xo::scm::IPrintable_DRuntimeError>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IPrintable_DRuntimeError
         **/
        class IPrintable_DRuntimeError {
        public:
            /** @defgroup scm-printable-druntimeerror-type-traits **/
            ///@{
            using ppindentinfo = xo::print::APrintable::ppindentinfo;
            using Copaque = xo::print::APrintable::Copaque;
            using Opaque = xo::print::APrintable::Opaque;
            ///@}
            /** @defgroup scm-printable-druntimeerror-methods **/
            ///@{
            // const methods
            /** Pretty-printing support for this object.  DEPRECATED:
the legacy two-pass ppindentinfo protocol, being replaced by
pretty(PpSink&).  See
.xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md
See [xo-indentlog/xo/indentlog/pretty.hpp] **/
            static bool pretty_deprecated(const DRuntimeError & self, const ppindentinfo & ppii);

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */
