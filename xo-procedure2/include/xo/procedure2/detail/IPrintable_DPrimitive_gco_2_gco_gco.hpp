/** @file IPrintable_DPrimitive_gco_2_gco_gco.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2-claude1/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DPrimitive_gco_2_gco_gco.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DPrimitive_gco_2_gco_gco.json5]
 **/

#pragma once

#include "Printable.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>
#include "DPrimitive_gco_2_gco_gco.hpp"

namespace xo { namespace scm { class IPrintable_DPrimitive_gco_2_gco_gco; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::print::APrintable,
                                   xo::scm::DPrimitive_gco_2_gco_gco>
        {
            using ImplType = xo::print::IPrintable_Xfer
              <xo::scm::DPrimitive_gco_2_gco_gco,
               xo::scm::IPrintable_DPrimitive_gco_2_gco_gco>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IPrintable_DPrimitive_gco_2_gco_gco
         **/
        class IPrintable_DPrimitive_gco_2_gco_gco {
        public:
            /** @defgroup scm-printable-dprimitive_gco_2_gco_gco-type-traits **/
            ///@{
            using ppindentinfo = xo::print::APrintable::ppindentinfo;
            using Copaque = xo::print::APrintable::Copaque;
            using Opaque = xo::print::APrintable::Opaque;
            ///@}
            /** @defgroup scm-printable-dprimitive_gco_2_gco_gco-methods **/
            ///@{
            // const methods
            /** Pretty-printing support for this object.
See [xo-indentlog/xo/indentlog/pretty.hpp] **/
            static bool pretty(const DPrimitive_gco_2_gco_gco & self, const ppindentinfo & ppii);

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */