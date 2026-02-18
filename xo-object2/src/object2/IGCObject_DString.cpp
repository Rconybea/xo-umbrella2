/** @file IGCObject_DString.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DString.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DString.json5]
**/

#include "string/IGCObject_DString.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DString::shallow_size(const DString & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DString::shallow_copy(const DString & self, obj<AAllocator> mm) noexcept -> Opaque
        {
            return self.shallow_copy(mm);
        }

        auto
        IGCObject_DString::forward_children(DString & self, obj<ACollector> gc) noexcept -> size_type
        {
            return self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DString.cpp */
