/** @file ICollector2_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Collector2.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Collector2.json5]
 **/

#pragma once

#include "ACollector2.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace mm { class ICollector2_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::mm::ACollector2,
                           DVariantPlaceholder>
{
    using ImplType = xo::mm::ICollector2_Any;
};

}
}

namespace xo {
namespace mm {

    /** @class ICollector2_Any
     *  @brief ACollector2 implementation for empty variant instance
     **/
    class ICollector2_Any : public ACollector2 {
    public:
        /** @defgroup mm-collector2-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using size_type = ACollector2::size_type;

        ///@}
        /** @defgroup mm-collector2-any-methods **/
        ///@{

        const ACollector2 * iface() const { return std::launder(this); }

        // from ACollector2

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void _drop(Opaque) const noexcept override { _fatal(); }

        // const methods
        [[noreturn]] size_type allocated(Copaque, Generation, role)  const  noexcept override { _fatal(); }
        [[noreturn]] size_type committed(Copaque, Generation, role)  const  noexcept override { _fatal(); }
        [[noreturn]] size_type reserved(Copaque, Generation, role)  const  noexcept override { _fatal(); }
        [[noreturn]] bool contains(Copaque, role, const void *)  const  noexcept override { _fatal(); }
        [[noreturn]] bool is_type_installed(Copaque, typeseq)  const  noexcept override { _fatal(); }

        // nonconst methods
        [[noreturn]] bool install_type(Opaque, const AGCObject &)  override;
        [[noreturn]] void add_gc_root_poly(Opaque, obj<AGCObject> *)  override;
        [[noreturn]] void remove_gc_root_poly(Opaque, obj<AGCObject> *)  override;
        [[noreturn]] void request_gc(Opaque, Generation)  override;
        [[noreturn]] void assign_member(Opaque, void *, obj<AGCObject> *, obj<AGCObject> &)  override;
        [[noreturn]] void forward_inplace(Opaque, AGCObject *, void **)  override;

        ///@}

    private:
        /** @defgraoup mm-collector2-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup mm-collector2-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace mm */
} /*namespace xo */

/* ICollector2_Any.hpp */
