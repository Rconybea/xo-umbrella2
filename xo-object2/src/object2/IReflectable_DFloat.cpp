/** @file IReflectable_DFloat.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IReflectable_DFloat.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IReflectable_DFloat.json5]
**/

#include "number/IReflectable_DFloat.hpp"

namespace xo {
    namespace scm {
        auto
        IReflectable_DFloat::self_tp(DFloat & self) -> TaggedPtr
        {
            return self.self_tp();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IReflectable_DFloat.cpp */
