/** @file IProcedure_DPrimitive_gco_2_dict_string.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IProcedure_DPrimitive_gco_2_dict_string.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IProcedure_DPrimitive_gco_2_dict_string.json5]
**/

#include "detail/IProcedure_DPrimitive_gco_2_dict_string.hpp"

namespace xo {
    namespace scm {
        auto
        IProcedure_DPrimitive_gco_2_dict_string::is_nary(const DPrimitive_gco_2_dict_string & self) noexcept -> bool
        {
            return self.is_nary();
        }

        auto
        IProcedure_DPrimitive_gco_2_dict_string::n_args(const DPrimitive_gco_2_dict_string & self) noexcept -> std::int32_t
        {
            return self.n_args();
        }

        auto
        IProcedure_DPrimitive_gco_2_dict_string::apply_nocheck(DPrimitive_gco_2_dict_string & self, obj<ARuntimeContext> rcx, const DArray * args) -> obj<AGCObject>
        {
            return self.apply_nocheck(rcx, args);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IProcedure_DPrimitive_gco_2_dict_string.cpp */
