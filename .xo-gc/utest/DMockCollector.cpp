/** @file DMockCollector.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "DMockCollector.hpp"

namespace xo {
    namespace mm {

        auto DMockCollector::allocated(Generation g, Role r) const noexcept -> size_type {
            return gcos_->allocated(g, r);
        }

        auto DMockCollector::committed(Generation g, Role r) const noexcept -> size_type {
            return gcos_->committed(g, r);
        }

        auto DMockCollector::reserved(Generation g, Role r) const noexcept -> size_type {
            return gcos_->reserved(g, r);
        }

        int32_t
        DMockCollector::locate_address(const void * addr) const noexcept {
            Generation g = gcos_->generation_of(Role::to_space(), addr);

            if (!g.is_sentinel())
                return g;

            return -1;
        }

        bool
        DMockCollector::contains(Role r, const void * addr) const noexcept {
            return gcos_->contains(r, addr);
        }

        bool
        DMockCollector::is_type_installed(typeseq tseq) const noexcept {
            return gcos_->is_type_installed(tseq);
        }

        bool
        DMockCollector::report_statistics(obj<AAllocator> mm,
                                          obj<AAllocator> error_mm,
                                          obj<AGCObject> * p_output) const noexcept
        {
            return false;
        }

        bool
        DMockCollector::report_object_types(obj<AAllocator> mm,
                                            obj<AAllocator> error_mm,
                                            obj<AGCObject> * p_output) const noexcept
        {
            return gcos_->report_object_types(mm, error_mm, p_output);
        }

        bool
        DMockCollector::report_object_ages(obj<AAllocator> mm,
                                            obj<AAllocator> error_mm,
                                            obj<AGCObject> * p_output) const noexcept
        {
            return gcos_->report_object_ages(mm, error_mm, p_output);
        }

        bool
        DMockCollector::install_type(const AGCObject & meta) noexcept
        {
            return gcos_->install_type(meta);
        }

        void
        DMockCollector::add_gc_root_poly(obj<AGCObject> * p_root)
        {
            assert(false);
        }

        void
        DMockCollector::remove_gc_root_poly(obj<AGCObject> * p_root)
        {
            assert(false);
        }

        void
        DMockCollector::request_gc(Generation upto)
        {
            assert(false);
        }

        void
        DMockCollector::assign_member(void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> & rhs)
        {
            mls_->assign_member_aux(gcos_,
                                    parent,
                                    p_lhs->iface(),
                                    p_lhs->opaque_data_addr(),
                                    rhs.iface(),
                                    rhs.opaque_data());
        }

        void *
        DMockCollector::alloc_copy(std::byte * src)
        {
            return gcos_->alloc_copy(src);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end DMockCollector.cpp */
