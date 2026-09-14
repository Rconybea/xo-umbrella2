/** @file ObjectHandle.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "AllocFlywheel.hpp"
#include <xo/refcnt/Displayable.hpp>

namespace xo::facet {

    /** @brief object handle for python bindings
     *
     *  Python bindings will use an ObjectHandle as a proxy for
     *  a particular underlying XO object.
     *
     *  In practice will inherit from this for each distinct interface.
     *
     *  AllocFlywheel stores anonymized obj<ATop> handles.
     *  ObjectHandle recovers a specific obj<Iface,DRepr> pair
     **/
    class ObjectHandleBase {
    public:
        using AllocFlywheel = xo::mm::AllocFlywheel;
        using FacetAppcx = xo::FacetAppcx;
        using Indentlog2Appcx = xo::Indentlog2Appcx;
        using impl_handle_type = AllocFlywheel::handle_type;
        using handle_index_type = AllocFlywheel::handle_index_type;

    public:
        /** @defgroup objecthandle-ctors **/
        ///@{
        ObjectHandleBase(rp<AllocFlywheel> mem,
                         std::pair<handle_index_type, impl_handle_type *> obj_info);

        ObjectHandleBase(ObjectHandleBase && other) noexcept;
        /** not copyable **/
        ObjectHandleBase(const ObjectHandleBase &) = delete;

        ~ObjectHandleBase();
        ///@}

        /** not assignable **/
        ObjectHandleBase & operator=(const ObjectHandleBase &) = delete;

        impl_handle_type * _impl_handle() const { return impl_handle_; }

        //const AllocFlywheel & flywheel() const { return *memory_.get(); }
        handle_index_type object_ix() const { return object_ix_; }

        /** @defgroup objecthandle-witness evidence carried by a handle
         *
         *  A handle cannot exist without a flywheel, which cannot exist without
         *  a FacetAppcx, which cannot exist without an Indentlog2Appcx.  These
         *  accessors make that chain reachable from the handle, so code that
         *  relies on it can say so (@ref xo::carries_facet_appcx) rather than
         *  asserting it in a comment.
         **/
        ///@{
        const FacetAppcx & facet_appcx() const { return memory_->facet_appcx(); }
        const Indentlog2Appcx & indentlog2_appcx() const { return memory_->indentlog2_appcx(); }
        ///@}

    private:
        rp<AllocFlywheel> memory_;
        /** identity of target object in @ref memory_ **/
        handle_index_type object_ix_;
        /** object handle **/
        impl_handle_type * impl_handle_ = nullptr;;
    };

    /** Provide a proxy for an @c obj<AFacet,DRepr> instance,
     *  where the instance belongs to a particular AllocFlywheel.
     **/
    template <typename AFacet, typename DRepr>
    class DObjectHandle : public ObjectHandleBase {
    public:
        using object_type = obj<AFacet, DRepr>;

    public:
        DObjectHandle(rp<AllocFlywheel> mem,
                      std::pair<handle_index_type, impl_handle_type *> obj_info)
        : ObjectHandleBase(mem, obj_info) {}

        /** move-only **/
        DObjectHandle(DObjectHandle &&) noexcept = default;

        /** Attach instance to flywheel @p mem to keep its backing memory alive.
         *
         *  Require: @p x was allocated from flywheel @p mem
         **/
        static DObjectHandle make_strong_ref(bp<AllocFlywheel> mem, obj<AFacet, DRepr> x) {
            /* Type-erased version of x.
             *
             * Not @code obj<ATop> impl_obj = x @endcode.
             * Relying on: flywheel slots are obj<ATop, ..>,
             * and AFacet inherits ATop.
             *
             * Necessary so polymorphic methods reach the right DRepr.
             */
            obj<ATop> impl_obj(static_cast<const ATop *>(x.iface()), x.opaque_data());

            auto ref = mem->add_strong_ref(impl_obj);

            return DObjectHandle(mem.promote(), ref);
        }

    public:
        /** recover the typed obj for this handle's target. **/
        object_type _native() const {
            return object_type(static_cast<DRepr *>(this->_impl_handle()->opaque_data()));
        }
    };
} /*namespace xo::facet*/

/* end ObjectHandle.hpp */
