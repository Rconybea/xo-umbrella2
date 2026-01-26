/** @file IRuntimeContext_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/RuntimeContext.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/RuntimeContext.json5]
 **/

#pragma once

#include "ARuntimeContext.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace scm { class IRuntimeContext_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::scm::ARuntimeContext,
                           DVariantPlaceholder>
{
    using ImplType = xo::scm::IRuntimeContext_Any;
};

}
}

namespace xo {
namespace scm {

    /** @class IRuntimeContext_Any
     *  @brief ARuntimeContext implementation for empty variant instance
     **/
    class IRuntimeContext_Any : public ARuntimeContext {
    public:
        /** @defgroup scm-runtimecontext-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using AAllocator = ARuntimeContext::AAllocator;

        ///@}
        /** @defgroup scm-runtimecontext-any-methods **/
        ///@{

        const ARuntimeContext * iface() const { return std::launder(this); }

        // from ARuntimeContext

        // const methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] obj<AAllocator> allocator(Copaque)  const  noexcept override { _fatal(); }

        // nonconst methods

        ///@}

    private:
        /** @defgraoup scm-runtimecontext-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup scm-runtimecontext-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace scm */
} /*namespace xo */

/* IRuntimeContext_Any.hpp */