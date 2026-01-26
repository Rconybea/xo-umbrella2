/** @file IProcedure_DPrimitive_gco_2_gco_gco.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2-claude1/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IProcedure_DPrimitive_gco_2_gco_gco.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IProcedure_DPrimitive_gco_2_gco_gco.json5]
 **/

#pragma once

#include "Procedure.hpp"
#include <xo/procedure2/RuntimeContext.hpp>
#include <xo/procedure2/detail/IRuntimeContext_Xfer.hpp>
#include <xo/procedure2/Procedure.hpp>
#include <xo/procedure2/detail/IProcedure_Xfer.hpp>
#include "DPrimitive_gco_2_gco_gco.hpp"

namespace xo { namespace scm { class IProcedure_DPrimitive_gco_2_gco_gco; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::scm::AProcedure,
                                   xo::scm::DPrimitive_gco_2_gco_gco>
        {
            using ImplType = xo::scm::IProcedure_Xfer
              <xo::scm::DPrimitive_gco_2_gco_gco,
               xo::scm::IProcedure_DPrimitive_gco_2_gco_gco>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IProcedure_DPrimitive_gco_2_gco_gco
         **/
        class IProcedure_DPrimitive_gco_2_gco_gco {
        public:
            /** @defgroup scm-procedure-dprimitive_gco_2_gco_gco-type-traits **/
            ///@{
            using AGCObject = xo::scm::AProcedure::AGCObject;
            using Copaque = xo::scm::AProcedure::Copaque;
            using Opaque = xo::scm::AProcedure::Opaque;
            ///@}
            /** @defgroup scm-procedure-dprimitive_gco_2_gco_gco-methods **/
            ///@{
            // const methods
            /** true iff procedure takes n arguments **/
            static bool is_nary(const DPrimitive_gco_2_gco_gco & self) noexcept;
            /** number of arguments. -1 for n-ary **/
            static std::int32_t n_args(const DPrimitive_gco_2_gco_gco & self) noexcept;

            // non-const methods
            /** invoke procedure; assume arguments satisfy type system **/
            static obj<AGCObject> apply_nocheck(DPrimitive_gco_2_gco_gco & self, obj<ARuntimeContext> rcx, const DArray * args);
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */