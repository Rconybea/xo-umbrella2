/** @file DMockCollector.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "MockCollector.hpp"

namespace xo {
    namespace mm {

        Generation
        DMockCollector::generation_of(Role r, const void * addr) const noexcept
        {
            return p_gco_store_->generation_of(r, addr);
        }

        AllocInfo
        DMockCollector::alloc_info(void * mem) const noexcept
        {
            return p_gco_store_->alloc_info((std::byte *)mem);
        }

        void
        DMockCollector::visit_child(AGCObject * lhs_iface, void ** lhs_data)
        {
            p_gco_store_->forward_inplace_aux(this->ref<AGCObjectVisitor>(), lhs_iface, lhs_data, upto_);
        }

        std::byte *
        DMockCollector::alloc_copy(void * src) noexcept {
            return p_gco_store_->new_space()->alloc_copy((std::byte *)src);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end DMockCollector.cpp */
