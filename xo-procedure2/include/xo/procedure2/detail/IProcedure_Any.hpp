/** @file IProcedure_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Procedure.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Procedure.json5]
 **/

#pragma once

#include "AProcedure.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace scm { class IProcedure_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::scm::AProcedure,
                           DVariantPlaceholder>
{
    using ImplType = xo::scm::IProcedure_Any;
};

}
}

namespace xo {
namespace scm {

    /** @class IProcedure_Any
     *  @brief AProcedure implementation for empty variant instance
     **/
    class IProcedure_Any : public AProcedure {
    public:
        /** @defgroup scm-procedure-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using AGCObject = AProcedure::AGCObject;

        ///@}
        /** @defgroup scm-procedure-any-methods **/
        ///@{

        const AProcedure * iface() const { return std::launder(this); }

        // from AProcedure

        // const methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] bool is_nary(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] std::int32_t n_args(Copaque)  const  noexcept override { _fatal(); }

        // nonconst methods
        [[noreturn]] obj<AGCObject> apply_nocheck(Opaque, const DArray *)  override;

        ///@}

    private:
        /** @defgraoup scm-procedure-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup scm-procedure-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace scm */
} /*namespace xo */

/* IProcedure_Any.hpp */