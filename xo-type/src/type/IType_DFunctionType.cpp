/** @file IType_DFunctionType.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IType_DFunctionType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IType_DFunctionType.json5]
**/

#include "function/IType_DFunctionType.hpp"

namespace xo {
    namespace scm {
        auto
        IType_DFunctionType::metatype(const DFunctionType & self) noexcept -> Metatype
        {
            return self.metatype();
        }

        auto
        IType_DFunctionType::repr_td(const DFunctionType & self) noexcept -> TypeDescr
        {
            return self.repr_td();
        }

        auto
        IType_DFunctionType::is_equal_to(const DFunctionType & self, const obj_AType & y) -> bool
        {
            return self.is_equal_to(y);
        }

        auto
        IType_DFunctionType::is_subtype_of(const DFunctionType & self, const obj_AType & y) -> bool
        {
            return self.is_subtype_of(y);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end IType_DFunctionType.cpp */
