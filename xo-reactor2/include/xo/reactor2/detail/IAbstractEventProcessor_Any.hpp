/** @file IAbstractEventProcessor_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/AbstractEventProcessor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/AbstractEventProcessor.json5]
 **/

#pragma once

#include "AAbstractEventProcessor.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace reactor { class IAbstractEventProcessor_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::reactor::AAbstractEventProcessor,
                           DVariantPlaceholder>
{
    using ImplType = xo::reactor::IAbstractEventProcessor_Any;
};

}
}

namespace xo {
namespace reactor {

    /** @class IAbstractEventProcessor_Any
     *  @brief AAbstractEventProcessor implementation for empty variant instance
     **/
    class IAbstractEventProcessor_Any : public AAbstractEventProcessor {
    public:
        /** @defgroup reactor-abstracteventprocessor-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using obj_AAbstractEventProcessor = AAbstractEventProcessor::obj_AAbstractEventProcessor;
        using DString = AAbstractEventProcessor::DString;

        ///@}
        /** @defgroup reactor-abstracteventprocessor-any-methods **/
        ///@{

        const AAbstractEventProcessor * iface() const { return std::launder(this); }

        // from AAbstractEventProcessor

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void _drop(Opaque) const noexcept override { _fatal(); }

        // const methods
        [[noreturn]] const DString * name(Copaque)  const  noexcept override { _fatal(); }

        // nonconst methods
        [[noreturn]] void set_name(Opaque, const DString *)  noexcept override;

        ///@}

    private:
        /** @defgraoup reactor-abstracteventprocessor-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup reactor-abstracteventprocessor-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace reactor */
} /*namespace xo */

/* IAbstractEventProcessor_Any.hpp */
