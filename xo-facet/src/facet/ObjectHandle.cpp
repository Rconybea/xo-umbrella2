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

    ObjectHandleBase::~ObjectHandleBase() = default;

} /*namespace xo::facet*/

/* end ObjectHandle.cpp */
