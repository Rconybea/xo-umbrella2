/** @file IReflectable_DString.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IReflectable_DString.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IReflectable_DString.json5]
**/

#include "string/IReflectable_DString.hpp"

namespace xo {
    namespace scm {
        auto
        IReflectable_DString::self_tp(DString & self) -> TaggedPtr
        {
            return self.self_tp();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IReflectable_DString.cpp */
