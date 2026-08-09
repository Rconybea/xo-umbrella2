/** @file IPrintable_DString.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DString.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DString.json5]
 **/

#pragma once

#include "DString.hpp"
#include "Printable.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>

namespace xo { namespace scm { class IPrintable_DString; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::print::APrintable,
                                   xo::scm::DString>
        {
            using ImplType = xo::print::IPrintable_Xfer
              <xo::scm::DString,
               xo::scm::IPrintable_DString>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IPrintable_DString
         **/
        class IPrintable_DString {
        public:
            /** @defgroup scm-printable-dstring-type-traits **/
            ///@{
            using ppindentinfo = xo::print::APrintable::ppindentinfo;
            using Copaque = xo::print::APrintable::Copaque;
            using Opaque = xo::print::APrintable::Opaque;
            ///@}
            /** @defgroup scm-printable-dstring-methods **/
            ///@{
            // const methods
            /** Pretty-printing support for this object.  DEPRECATED:
the legacy two-pass ppindentinfo protocol, being replaced by
pretty(PpSink&).  See
.xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md
See [xo-indentlog/xo/indentlog/pretty.hpp] **/
            static bool pretty_deprecated(const DString & self, const ppindentinfo & ppii);

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */
