/** @file DGCObjectStoreVisitor.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GCObjectStoreVisitor.hpp"
#include "GCObjectStore.hpp"

namespace xo {
    namespace mm {

        DGCObjectStoreVisitor::DGCObjectStoreVisitor(GCObjectStore * gcos,
                                                     Generation upto)
        : p_gco_store_{gcos}, upto_{upto}
        {}

        Generation
        DGCObjectStoreVisitor::generation_of(Role r, const void * addr) const noexcept
        {
            return p_gco_store_->generation_of(r, addr);
        }

        AllocInfo
        DGCObjectStoreVisitor::alloc_info(void * mem) const noexcept
        {
            return p_gco_store_->alloc_info((std::byte *)mem);
        }

        void
        DGCObjectStoreVisitor::visit_child(VisitReason reason,
                                           AGCObject * lhs_iface, void ** lhs_data)
        {
            switch (reason.code()) {
            case VisitReason::code::forward:
                p_gco_store_->_forward_inplace_aux
                    (this->ref<AGCObjectVisitor>(), lhs_iface, lhs_data, upto_);
                break;
            case VisitReason::code::verify:
                p_gco_store_->_verify_aux(lhs_iface, *lhs_data);
                break;
            default:
                assert(false);
            }
        }

        std::byte *
        DGCObjectStoreVisitor::alloc_copy(void * src) noexcept {
            // check whether we're promoting src.

            return p_gco_store_->alloc_copy((std::byte *)src);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end DGCObjectVisitor.cpp */
