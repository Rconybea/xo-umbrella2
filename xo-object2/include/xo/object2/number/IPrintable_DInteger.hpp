/** @file IPrintable_DInteger.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2-claude1/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DInteger.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DInteger.json5]
 **/

#pragma once

#include "Printable.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>
#include "DInteger.hpp"

namespace xo { namespace scm { class IPrintable_DInteger; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::print::APrintable,
                                   xo::scm::DInteger>
        {
            using ImplType = xo::print::IPrintable_Xfer
              <xo::scm::DInteger,
               xo::scm::IPrintable_DInteger>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IPrintable_DInteger
         **/
        class IPrintable_DInteger {
        public:
            /** @defgroup scm-printable-dinteger-type-traits **/
            ///@{
            using ppindentinfo = xo::print::APrintable::ppindentinfo;
            using Copaque = xo::print::APrintable::Copaque;
            using Opaque = xo::print::APrintable::Opaque;
            ///@}
            /** @defgroup scm-printable-dinteger-methods **/
            ///@{
            // const methods
            /** Pretty-printing support for this object.
See [xo-indentlog/xo/indentlog/pretty.hpp] **/
            static bool pretty(const DInteger & self, const ppindentinfo & ppii);

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */