/** @file IGCObject_DFunctionType.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DFunctionType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DFunctionType.json5]
**/

#include "function/IGCObject_DFunctionType.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DFunctionType::shallow_size(const DFunctionType & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DFunctionType::shallow_copy(const DFunctionType & self, obj<AAllocator> mm) noexcept -> Opaque
        {
            return self.shallow_copy(mm);
        }

        auto
        IGCObject_DFunctionType::forward_children(DFunctionType & self, obj<ACollector> gc) noexcept -> size_type
        {
            return self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DFunctionType.cpp */
