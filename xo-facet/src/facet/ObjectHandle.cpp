/** @file ObjectHandle.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "ObjectHandle.hpp"

namespace xo::facet {

    ObjectHandleBase::ObjectHandleBase(rp<AllocFlywheel> mem,
                                       std::pair<handle_index_type, impl_handle_type *> obj_info)
        : memory_{std::move(mem)}, object_ix_{obj_info.first}, impl_handle_{obj_info.second}
    {}

    ObjectHandleBase::ObjectHandleBase(ObjectHandleBase && other) noexcept
        : memory_{std::move(other.memory_)},
          object_ix_{other.object_ix_},
          impl_handle_{other.impl_handle_}
    {
        /* the moved-from handle must not release: one slot, one owner.
         * memory_ is already null after the move, which is what ~ObjectHandleBase
         * tests, but clearing impl_handle_ too keeps the empty state readable.
         */
        other.impl_handle_ = nullptr;
    }

    ObjectHandleBase::~ObjectHandleBase()
    {
        /* null after a move-from, and only then */
        if (memory_)
            memory_->remove_strong_ref(object_ix_);
    }

} /*namespace xo::facet*/

/* end ObjectHandle.cpp */
