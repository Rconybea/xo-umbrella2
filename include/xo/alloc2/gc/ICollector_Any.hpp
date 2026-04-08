/** @file ICollector_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Collector.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Collector.json5]
 **/

#pragma once

#include "ACollector.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace mm { class ICollector_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::mm::ACollector,
                           DVariantPlaceholder>
{
    using ImplType = xo::mm::ICollector_Any;
};

}
}

namespace xo {
namespace mm {

    /** @class ICollector_Any
     *  @brief ACollector implementation for empty variant instance
     **/
    class ICollector_Any : public ACollector {
    public:
        /** @defgroup mm-collector-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using size_type = ACollector::size_type;

        ///@}
        /** @defgroup mm-collector-any-methods **/
        ///@{

        const ACollector * iface() const { return std::launder(this); }

        // from ACollector

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void _drop(Opaque) const noexcept override { _fatal(); }

        // const methods
        [[noreturn]] size_type allocated(Copaque, Generation, Role)  const  noexcept override { _fatal(); }
        [[noreturn]] size_type committed(Copaque, Generation, Role)  const  noexcept override { _fatal(); }
        [[noreturn]] size_type reserved(Copaque, Generation, Role)  const  noexcept override { _fatal(); }
        [[noreturn]] std::int32_t locate_address(Copaque, const void *)  const  noexcept override { _fatal(); }
        [[noreturn]] bool contains(Copaque, Role, const void *)  const  noexcept override { _fatal(); }
        [[noreturn]] bool is_type_installed(Copaque, typeseq)  const  noexcept override { _fatal(); }
        [[noreturn]] bool report_statistics(Copaque, obj<AAllocator>, obj<AAllocator>, obj<AGCObject> *)  const  noexcept override { _fatal(); }
        [[noreturn]] bool report_object_types(Copaque, obj<AAllocator>, obj<AAllocator>, obj<AGCObject> *)  const  noexcept override { _fatal(); }
        [[noreturn]] bool report_object_ages(Copaque, obj<AAllocator>, obj<AAllocator>, obj<AGCObject> *)  const  noexcept override { _fatal(); }

        // nonconst methods
        [[noreturn]] bool install_type(Opaque, const AGCObject &)  override;
        [[noreturn]] void add_gc_root_poly(Opaque, obj<AGCObject> *)  override;
        [[noreturn]] void remove_gc_root_poly(Opaque, obj<AGCObject> *)  override;
        [[noreturn]] void request_gc(Opaque, Generation)  override;
        [[noreturn]] void assign_member(Opaque, void *, obj<AGCObject> *, obj<AGCObject> &)  override;
        [[noreturn]] void * alloc_copy(Opaque, std::byte *)  override;

        ///@}

    private:
        /** @defgraoup mm-collector-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup mm-collector-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace mm */
} /*namespace xo */

/* ICollector_Any.hpp */
