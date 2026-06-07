/** @file ICollector_DMockCollector.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ICollector_DMockCollector.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ICollector_DMockCollector.json5]
**/

#include "detail/ICollector_DMockCollector.hpp"

namespace xo {
    namespace mm {
        auto
        ICollector_DMockCollector::allocated(const DMockCollector & self, Generation g, Role r) noexcept -> size_type
        {
            return self.allocated(g, r);
        }

        auto
        ICollector_DMockCollector::committed(const DMockCollector & self, Generation g, Role r) noexcept -> size_type
        {
            return self.committed(g, r);
        }

        auto
        ICollector_DMockCollector::reserved(const DMockCollector & self, Generation g, Role r) noexcept -> size_type
        {
            return self.reserved(g, r);
        }

        auto
        ICollector_DMockCollector::locate_address(const DMockCollector & self, const void * addr) noexcept -> std::int32_t
        {
            return self.locate_address(addr);
        }

        auto
        ICollector_DMockCollector::contains(const DMockCollector & self, Role r, const void * addr) noexcept -> bool
        {
            return self.contains(r, addr);
        }

        auto
        ICollector_DMockCollector::is_type_installed(const DMockCollector & self, typeseq tseq) noexcept -> bool
        {
            return self.is_type_installed(tseq);
        }

        auto
        ICollector_DMockCollector::report_statistics(const DMockCollector & self, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output) noexcept -> bool
        {
            return self.report_statistics(report_mm, error_mm, output);
        }

        auto
        ICollector_DMockCollector::report_object_types(const DMockCollector & self, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output) noexcept -> bool
        {
            return self.report_object_types(report_mm, error_mm, output);
        }

        auto
        ICollector_DMockCollector::report_object_ages(const DMockCollector & self, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output) noexcept -> bool
        {
            return self.report_object_ages(report_mm, error_mm, output);
        }

        auto
        ICollector_DMockCollector::install_type(DMockCollector & self, const AGCObject & iface) -> bool
        {
            return self.install_type(iface);
        }
        auto
        ICollector_DMockCollector::add_gc_root_poly(DMockCollector & self, obj<AGCObject> * p_root) -> void
        {
            self.add_gc_root_poly(p_root);
        }
        auto
        ICollector_DMockCollector::remove_gc_root_poly(DMockCollector & self, obj<AGCObject> * p_root) -> void
        {
            self.remove_gc_root_poly(p_root);
        }
        auto
        ICollector_DMockCollector::request_gc(DMockCollector & self, Generation upto) -> void
        {
            self.request_gc(upto);
        }
        auto
        ICollector_DMockCollector::assign_member(DMockCollector & self, void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> & rhs) -> void
        {
            self.assign_member(parent, p_lhs, rhs);
        }
        auto
        ICollector_DMockCollector::alloc_copy(DMockCollector & self, std::byte * src) -> void *
        {
            return self.alloc_copy(src);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end ICollector_DMockCollector.cpp */
