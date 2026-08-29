/** @file IPrintable_DObjectEvent.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IPrintable_DObjectEvent.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IPrintable_DObjectEvent.json5]
**/

#include "event/IPrintable_DObjectEvent.hpp"

namespace xo {
    namespace process {
        auto
        IPrintable_DObjectEvent::pretty(const DObjectEvent & self, PpSink & sink) -> void
        {
            self.pretty(sink);
        }


    } /*namespace process*/
} /*namespace xo*/

/* end IPrintable_DObjectEvent.cpp */
