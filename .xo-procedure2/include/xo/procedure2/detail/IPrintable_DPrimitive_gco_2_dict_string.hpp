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

#include "Printable.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>
#include "DPrimitive_gco_2_dict_string.hpp"

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
            using Copaque = xo::print::APrintable::Copaque;
            using Opaque = xo::print::APrintable::Opaque;
            ///@}
            /** @defgroup scm-printable-dprimitive_gco_2_dict_string-methods **/
            ///@{
            // const methods
            /** Pretty-printing support for this object.
See [xo-indentlog/xo/indentlog/pretty.hpp] **/
            static bool pretty(const DPrimitive_gco_2_dict_string & self, const ppindentinfo & ppii);

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */