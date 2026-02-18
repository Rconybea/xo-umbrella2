/** @file IProcedure_DPrimitive_gco_2_gco_gco.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IProcedure_DPrimitive_gco_2_gco_gco.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IProcedure_DPrimitive_gco_2_gco_gco.json5]
**/

#include "detail/IProcedure_DPrimitive_gco_2_gco_gco.hpp"

namespace xo {
    namespace scm {
        auto
        IProcedure_DPrimitive_gco_2_gco_gco::is_nary(const DPrimitive_gco_2_gco_gco & self) noexcept -> bool
        {
            return self.is_nary();
        }

        auto
        IProcedure_DPrimitive_gco_2_gco_gco::n_args(const DPrimitive_gco_2_gco_gco & self) noexcept -> std::int32_t
        {
            return self.n_args();
        }

        auto
        IProcedure_DPrimitive_gco_2_gco_gco::apply_nocheck(DPrimitive_gco_2_gco_gco & self, obj<ARuntimeContext> rcx, const DArray * args) -> obj<AGCObject>
        {
            return self.apply_nocheck(rcx, args);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IProcedure_DPrimitive_gco_2_gco_gco.cpp */
