/** @file IType_DTypeVarRef.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IType_DTypeVarRef.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IType_DTypeVarRef.json5]
**/

#include "typevar/IType_DTypeVarRef.hpp"

namespace xo {
    namespace scm {
        auto
        IType_DTypeVarRef::metatype(const DTypeVarRef & self) noexcept -> Metatype
        {
            return self.metatype();
        }

        auto
        IType_DTypeVarRef::repr_td(const DTypeVarRef & self) noexcept -> TypeDescr
        {
            return self.repr_td();
        }

        auto
        IType_DTypeVarRef::is_equal_to(const DTypeVarRef & self, const obj_AType & y) -> bool
        {
            return self.is_equal_to(y);
        }

        auto
        IType_DTypeVarRef::is_subtype_of(const DTypeVarRef & self, const obj_AType & y) -> bool
        {
            return self.is_subtype_of(y);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end IType_DTypeVarRef.cpp */
