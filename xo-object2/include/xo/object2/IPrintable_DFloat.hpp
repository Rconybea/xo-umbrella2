/** @file IPrintable_DFloat.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DFloat.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DFloat.json5]
 **/

#pragma once

#include <xo/printable2/Printable.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>
#include "DFloat.hpp"

namespace xo { namespace scm { class IPrintable_DFloat; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::print::APrintable,
                                   xo::scm::DFloat>
        {
            using ImplType = xo::print::IPrintable_Xfer
              <xo::scm::DFloat,
               xo::scm::IPrintable_DFloat>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IPrintable_DFloat
         **/
        class IPrintable_DFloat {
        public:
            /** @defgroup scm-printable-dfloat-type-traits **/
            ///@{
            using ppindentinfo = xo::print::APrintable::ppindentinfo;
            ///@}
            /** @defgroup scm-printable-dfloat-methods **/
            ///@{
            /** Pretty-printing support for this object.
See [xo-indentlog/xo/indentlog/pretty.hpp] **/
            static bool pretty(const DFloat & self, const ppindentinfo & ppii);

            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */