/** @file IGCObject_DRuntimeError.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DRuntimeError.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DRuntimeError.json5]
**/

#include "error/IGCObject_DRuntimeError.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DRuntimeError::shallow_size(const DRuntimeError & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DRuntimeError::shallow_copy(const DRuntimeError & self, obj<AAllocator> mm) noexcept -> Opaque
        {
            return self.shallow_copy(mm);
        }

        auto
        IGCObject_DRuntimeError::forward_children(DRuntimeError & self, obj<ACollector> gc) noexcept -> size_type
        {
            return self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DRuntimeError.cpp */
