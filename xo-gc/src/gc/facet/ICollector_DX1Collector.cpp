/** @file ICollector_DX1Collector.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ICollector_DX1Collector.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ICollector_DX1Collector.json5]
**/

#include "detail/ICollector_DX1Collector.hpp"

namespace xo {
    namespace mm {
        auto
        ICollector_DX1Collector::allocated(const DX1Collector & self, Generation g, Role r) noexcept -> size_type
        {
            return self.allocated(g, r);
        }

        auto
        ICollector_DX1Collector::committed(const DX1Collector & self, Generation g, Role r) noexcept -> size_type
        {
            return self.committed(g, r);
        }

        auto
        ICollector_DX1Collector::reserved(const DX1Collector & self, Generation g, Role r) noexcept -> size_type
        {
            return self.reserved(g, r);
        }

        auto
        ICollector_DX1Collector::locate_address(const DX1Collector & self, const void * addr) noexcept -> std::int32_t
        {
            return self.locate_address(addr);
        }

        auto
        ICollector_DX1Collector::contains(const DX1Collector & self, Role r, const void * addr) noexcept -> bool
        {
            return self.contains(r, addr);
        }

        auto
        ICollector_DX1Collector::is_type_installed(const DX1Collector & self, typeseq tseq) noexcept -> bool
        {
            return self.is_type_installed(tseq);
        }

        auto
        ICollector_DX1Collector::report_statistics(const DX1Collector & self, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output) noexcept -> bool
        {
            return self.report_statistics(report_mm, error_mm, output);
        }

        auto
        ICollector_DX1Collector::report_object_types(const DX1Collector & self, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output) noexcept -> bool
        {
            return self.report_object_types(report_mm, error_mm, output);
        }

        auto
        ICollector_DX1Collector::report_object_ages(const DX1Collector & self, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output) noexcept -> bool
        {
            return self.report_object_ages(report_mm, error_mm, output);
        }

        auto
        ICollector_DX1Collector::install_type(DX1Collector & self, const AGCObject & iface) -> bool
        {
            return self.install_type(iface);
        }
        auto
        ICollector_DX1Collector::add_gc_root_poly(DX1Collector & self, obj<AGCObject> * p_root) -> void
        {
            self.add_gc_root_poly(p_root);
        }
        auto
        ICollector_DX1Collector::remove_gc_root_poly(DX1Collector & self, obj<AGCObject> * p_root) -> void
        {
            self.remove_gc_root_poly(p_root);
        }
        auto
        ICollector_DX1Collector::request_gc(DX1Collector & self, Generation upto) -> void
        {
            self.request_gc(upto);
        }
        auto
        ICollector_DX1Collector::assign_member(DX1Collector & self, void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> & rhs) -> void
        {
            self.assign_member(parent, p_lhs, rhs);
        }
        auto
        ICollector_DX1Collector::alloc_copy(DX1Collector & self, std::byte * src) -> void *
        {
            return self.alloc_copy(src);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end ICollector_DX1Collector.cpp */
