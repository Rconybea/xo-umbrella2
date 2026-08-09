/** @file IPrintable_DPrimitive_gco_2_dict_string.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DPrimitive_gco_2_dict_string.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DPrimitive_gco_2_dict_string.json5]
 **/

#pragma once

#include "DPrimitive_gco_2_dict_string.hpp"
#include "Printable.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>

namespace xo { namespace scm { class IPrintable_DPrimitive_gco_2_dict_string; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::print::APrintable,
                                   xo::scm::DPrimitive_gco_2_dict_string>
        {
            using ImplType = xo::print::IPrintable_Xfer
              <xo::scm::DPrimitive_gco_2_dict_string,
               xo::scm::IPrintable_DPrimitive_gco_2_dict_string>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IPrintable_DPrimitive_gco_2_dict_string
         **/
        class IPrintable_DPrimitive_gco_2_dict_string {
        public:
            /** @defgroup scm-printable-dprimitive_gco_2_dict_string-type-traits **/
            ///@{
            using ppindentinfo = xo::print::APrintable::ppindentinfo;
            using PpSink = xo::print::APrintable::PpSink;
            using Copaque = xo::print::APrintable::Copaque;
            using Opaque = xo::print::APrintable::Opaque;
            ///@}
            /** @defgroup scm-printable-dprimitive_gco_2_dict_string-methods **/
            ///@{
            // const methods
            /** Pretty-printing support for this object.  DEPRECATED:
the legacy two-pass ppindentinfo protocol, being replaced by
pretty(PpSink&).  See
.xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md
See [xo-indentlog/xo/indentlog/pretty.hpp] **/
            static bool pretty_deprecated(const DPrimitive_gco_2_dict_string & self, const ppindentinfo & ppii);
            /** Pretty-printing support for this object.
Single-pass: render into @p sink.  A nested object
participates in the enclosing structure's line breaking,
where the deprecated two-pass protocol could not.
See [xo-ppsink/xo/ppsink/PpSink.hpp] **/
            static void pretty(const DPrimitive_gco_2_dict_string & self, PpSink & sink);

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */
