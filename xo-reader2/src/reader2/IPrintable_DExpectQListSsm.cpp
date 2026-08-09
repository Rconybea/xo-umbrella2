/** @file IPrintable_DExpectQListSsm.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DExpectQListSsm.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DExpectQListSsm.json5]
**/

#include "ssm/IPrintable_DExpectQListSsm.hpp"

namespace xo {
    namespace scm {
        auto
        IPrintable_DExpectQListSsm::pretty_deprecated(const DExpectQListSsm & self, const ppindentinfo & ppii) -> bool
        {
            return self.pretty_deprecated(ppii);
        }

        auto
        IPrintable_DExpectQListSsm::pretty(const DExpectQListSsm & self, PpSink & sink) -> void
        {
            self.pretty(sink);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end IPrintable_DExpectQListSsm.cpp */
