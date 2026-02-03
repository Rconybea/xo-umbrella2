/** @file IResourceVisitor_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ResourceVisitor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ResourceVisitor.json5]
 **/

#pragma once

#include "AResourceVisitor.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace mm { class IResourceVisitor_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::mm::AResourceVisitor,
                           DVariantPlaceholder>
{
    using ImplType = xo::mm::IResourceVisitor_Any;
};

}
}

namespace xo {
namespace mm {

    /** @class IResourceVisitor_Any
     *  @brief AResourceVisitor implementation for empty variant instance
     **/
    class IResourceVisitor_Any : public AResourceVisitor {
    public:
        /** @defgroup mm-resourcevisitor-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using size_type = AResourceVisitor::size_type;

        ///@}
        /** @defgroup mm-resourcevisitor-any-methods **/
        ///@{

        const AResourceVisitor * iface() const { return std::launder(this); }

        // from AResourceVisitor

        // const methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void on_allocator(Copaque, obj<AAllocator>)  const  noexcept override { _fatal(); }

        // nonconst methods

        ///@}

    private:
        /** @defgraoup mm-resourcevisitor-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup mm-resourcevisitor-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace mm */
} /*namespace xo */

/* IResourceVisitor_Any.hpp */
