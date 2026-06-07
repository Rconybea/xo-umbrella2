/** @file IType_DArrayType.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IType_DArrayType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IType_DArrayType.json5]
**/

#include "array/IType_DArrayType.hpp"

namespace xo {
    namespace scm {
        auto
        IType_DArrayType::metatype(const DArrayType & self) noexcept -> Metatype
        {
            return self.metatype();
        }

        auto
        IType_DArrayType::repr_td(const DArrayType & self) noexcept -> TypeDescr
        {
            return self.repr_td();
        }

        auto
        IType_DArrayType::is_equal_to(const DArrayType & self, const obj_AType & y) -> bool
        {
            return self.is_equal_to(y);
        }

        auto
        IType_DArrayType::is_subtype_of(const DArrayType & self, const obj_AType & y) -> bool
        {
            return self.is_subtype_of(y);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end IType_DArrayType.cpp */
