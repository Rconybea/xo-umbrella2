/** @file ISequence_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2-claude1/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Sequence.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Sequence.json5]
 **/

#pragma once

#include "ASequence.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace scm { class ISequence_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::scm::ASequence,
                           DVariantPlaceholder>
{
    using ImplType = xo::scm::ISequence_Any;
};

}
}

namespace xo {
namespace scm {

    /** @class ISequence_Any
     *  @brief ASequence implementation for empty variant instance
     **/
    class ISequence_Any : public ASequence {
    public:
        /** @defgroup scm-sequence-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using size_type = ASequence::size_type;
        using AGCObject = ASequence::AGCObject;

        ///@}
        /** @defgroup scm-sequence-any-methods **/
        ///@{

        const ASequence * iface() const { return std::launder(this); }

        // from ASequence

        // const methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] bool is_empty(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] bool is_finite(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] obj<AGCObject> at(Copaque, size_type)  const override { _fatal(); }

        // nonconst methods

        ///@}

    private:
        /** @defgraoup scm-sequence-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup scm-sequence-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace scm */
} /*namespace xo */

/* ISequence_Any.hpp */