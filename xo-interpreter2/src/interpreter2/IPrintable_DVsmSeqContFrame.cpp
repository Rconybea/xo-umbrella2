/** @file IPrintable_DVsmSeqContFrame.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DVsmSeqContFrame.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DVsmSeqContFrame.json5]
**/

#include "sequence/IPrintable_DVsmSeqContFrame.hpp"

namespace xo {
    namespace scm {
        auto
        IPrintable_DVsmSeqContFrame::pretty_deprecated(const DVsmSeqContFrame & self, const ppindentinfo & ppii) -> bool
        {
            return self.pretty_deprecated(ppii);
        }

        auto
        IPrintable_DVsmSeqContFrame::pretty(const DVsmSeqContFrame & self, PpSink & sink) -> void
        {
            self.pretty(sink);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end IPrintable_DVsmSeqContFrame.cpp */
