/** @file CallbackSet.hpp
 *
 *  Arena-allocated set of callbacks, with explicit obj<AAllocator>.
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include "callback_traits.hpp"
#include <xo/callback/CallbackId.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/obj.hpp>
#include <cstdint>
#include <new>

namespace xo {
    namespace fn {
        /** one registered callback **/
        template <typename Callback>
        struct CbRecd {
            CbRecd() = default;
            CbRecd(CallbackId id, Callback fn) : id_{id}, fn_{fn} {}

            /** identifies this callback, for remove_callback() **/
            CallbackId id_;
            /** the callback itself **/
            Callback fn_;
            /** for a slot in the deferred region: true iff a deferred add.
             *  unused for live slots.
             **/
            bool is_add_ = false;
        }; /*CbRecd*/

        /** @class CallbackSet
         *  @brief set of callbacks, allocated from an explicit obj<AAllocator>
         *
         *  @tp Callback  either a faceted obj<AFacet,DRepr>
         *                or a plain function pointer.
         *                std::function and lambdas rejected, because for faceted
         *                objects we need to support AGCObject.
         *
         *  Storage follows the DArray idiom: capacity is fixed at allocation,
         *  add_callback() fails when full, and grow() is explicit via copy().
         *  There is no reallocation, so nothing is ever stranded in the arena.
         *
         *  Reentrant: callbacks running under invoke()/invoke_each() may add or
         *  remove callbacks on this set; such changes are deferred to the end of
         *  the run.  The deferral region has its own fixed capacity.
         *
         *  Not threadsafe.
         **/
        template <typename Callback>
        class CallbackSet {
        public:
            using size_type = std::uint32_t;
            using callback_type = Callback;
            using slot_type = CbRecd<Callback>;

            static_assert(!is_std_function_v<Callback>,
                          "xo::fn::CallbackSet: std::function is not supported."
                          " It allocates from the heap (its allocator-extended"
                          " ctors were removed in c++17), and hides captured"
                          " references from the collector."
                          " Use obj<AFacet> or a plain function pointer.");

            static_assert(callback2_concept<Callback>,
                          "xo::fn::CallbackSet: unsupported callback type."
                          " Expected obj<AFacet,DRepr> (traced) or a plain"
                          " function pointer (untraced).  Closure types"
                          " (lambdas) are excluded: they can capture an"
                          " obj<AFacet> that the collector cannot see."
                          " Pass a lambda to invoke_each() instead -- that one"
                          " is transient, not stored.");

            /** true iff callbacks in this set can hold traced references **/
            static constexpr bool is_traced = fomo_callback_concept<Callback>;

        public:
            /** create empty set with room for @p cap callbacks and @p defer_cap
             *  deferred add/remove commands, using memory from @p mm.
             *  Nullptr if space exhausted.
             **/
            static CallbackSet * _empty(xo::facet::obj<xo::mm::AAllocator> mm,
                                        size_type cap,
                                        size_type defer_cap)
                {
                    CallbackSet * result = nullptr;

                    void * mem
                        = mm.alloc(xo::reflect::typeseq::id<CallbackSet>(),
                                   (sizeof(CallbackSet)
                                    + (cap + defer_cap) * sizeof(slot_type)));

                    if (mem) [[likely]] {
                        result = new (mem) CallbackSet();

                        result->capacity_ = cap;
                        result->defer_capacity_ = defer_cap;
                    }

                    return result;
                }

            /** create empty set, deferral capacity defaulting to @p cap **/
            static CallbackSet * _empty(xo::facet::obj<xo::mm::AAllocator> mm,
                                        size_type cap)
                {
                    return _empty(mm, cap, cap);
                }

            /** copy of @p src with capacity for @p new_cap callbacks.
             *  This is how a set grows.  Nullptr if space exhausted.
             *
             *  Require: src has no deferred commands pending (i.e. is not
             *  running), and new_cap >= src->size()
             **/
            static CallbackSet * copy(xo::facet::obj<xo::mm::AAllocator> mm,
                                      const CallbackSet * src,
                                      size_type new_cap)
                {
                    if (!src || (new_cap < src->size_))
                        return nullptr;

                    CallbackSet * result = _empty(mm, new_cap,
                                                  src->defer_capacity_);

                    if (result) [[likely]] {
                        for (size_type i = 0; i < src->size_; ++i)
                            result->slot_[i] = src->slot_[i];

                        result->size_ = src->size_;
                    }

                    return result;
                }

            /** @defgroup cbset-access access **/
            ///@{
            bool is_empty() const noexcept { return size_ == 0; }
            size_type capacity() const noexcept { return capacity_; }
            size_type size() const noexcept { return size_; }
            bool is_full() const noexcept { return size_ >= capacity_; }
            /** true while callbacks from this set are running **/
            bool is_running() const noexcept { return running_; }

            const slot_type * begin() const noexcept { return slot_; }
            const slot_type * end() const noexcept { return slot_ + size_; }
            ///@}

            /** @defgroup cbset-modify add/remove **/
            ///@{

            /** register @p fn.  Returns its id, or a default-constructed
             *  CallbackId if the set (or, while running, the deferral region)
             *  is full.
             *
             *  Reentrant: safe to call from a running callback.
             **/
            CallbackId add_callback(Callback fn) noexcept {
                CallbackId id = CallbackId::generate();

                if (running_) {
                    if (n_defer_ >= defer_capacity_)
                        return CallbackId();

                    slot_type & s = slot_[capacity_ + n_defer_];

                    s = slot_type(id, fn);
                    s.is_add_ = true;

                    ++n_defer_;
                } else {
                    if (this->is_full())
                        return CallbackId();

                    slot_[size_] = slot_type(id, fn);

                    ++size_;
                }

                return id;
            }

            /** unregister the callback with id @p id.  No-op if absent.
             *  Reentrant.
             **/
            bool remove_callback(CallbackId id) noexcept {
                if (running_) {
                    if (n_defer_ >= defer_capacity_)
                        return false;

                    slot_type & s = slot_[capacity_ + n_defer_];

                    s = slot_type(id, Callback());
                    s.is_add_ = false;

                    ++n_defer_;

                    return true;
                }

                return this->remove_callback_impl(id);
            }
            ///@}

            /** @defgroup cbset-invoke invocation **/
            ///@{

            /** call fn(cb, args...) for each registered callback cb.
             *
             *  @p fn is not stored, so a lambda is fine here
             *  lambda is fine here, and is the idiomatic way to name a facet
             *  method:
             *
             *    cbset.invoke_each([](auto & cb, const Event & ev) {
             *                          cb.notify_event(ev);
             *                      },
             *                      ev);
             **/
            template <typename Fn, typename... Tn>
            void invoke_each(Fn && fn, Tn&&... args) {
                running_ = true;

                try {
                    for (size_type i = 0; i < size_; ++i)
                        fn(slot_[i].fn_, args...);

                    this->apply_deferred();
                } catch(...) {
                    this->apply_deferred();
                    throw;
                }
            }

            /** call (cb.*member_fn)(args...) for each registered callback.
             *
             *  For faceted callbacks, @p member_fn is simply
             *  &Callback::some_facet_method
             *  (a pointer-to-member of the router base converts implicitly).
             *
             *  Use:
             *    cbset.invoke(&Callback::notify_event, ev);
             *
             *  MemberFn is deduced whole, so const/noexcept overloads and
             *  methods inherited from any base all work.
             **/
            template <typename MemberFn, typename... Tn>
                requires fomo_callback_concept<Callback>
            void invoke(MemberFn member_fn, Tn&&... args) {
                this->invoke_each([member_fn](Callback & cb, auto&&... inner) {
                                      (cb.*member_fn)(inner...);
                                  },
                                  args...);
            }

            /** call cb(args...) for each registered callback.
             *  For plain function-pointer callbacks.
             **/
            template <typename... Tn>
                requires plain_callback_concept<Callback>
            void invoke(Tn&&... args) {
                this->invoke_each([](Callback & cb, auto&&... inner) {
                                      cb(inner...);
                                  },
                                  args...);
            }
            ///@}

        private:
            CallbackSet() = default;

            /** remove callback @p id from the live region **/
            bool remove_callback_impl(CallbackId id) noexcept {
                for (size_type i = 0; i < size_; ++i) {
                    if (slot_[i].id_ == id) {
                        /* order is not significant: fill the hole from the end */
                        slot_[i] = slot_[size_ - 1];

                        --size_;

                        return true;
                    }
                }

                return false;
            }

            /** apply add/remove commands deferred during a callback run **/
            void apply_deferred() noexcept {
                running_ = false;

                for (size_type i = 0; i < n_defer_; ++i) {
                    const slot_type & s = slot_[capacity_ + i];

                    if (s.is_add_) {
                        if (!this->is_full()) {
                            slot_[size_] = s;
                            slot_[size_].is_add_ = false;

                            ++size_;
                        }
                    } else {
                        this->remove_callback_impl(s.id_);
                    }
                }

                n_defer_ = 0;
            }

        private:
            /** extent of the live region of @ref slot_ **/
            size_type capacity_ = 0;
            /** number of live callbacks.  Invariant: size_ <= capacity_ **/
            size_type size_ = 0;
            /** extent of the deferral region, which begins at slot_[capacity_] **/
            size_type defer_capacity_ = 0;
            /** number of pending deferred commands **/
            size_type n_defer_ = 0;
            /** true while callbacks are running; add/remove defer in that state **/
            bool running_ = false;
            /** live callbacks in [0, size_);
             *  deferred commands in [capacity_, capacity_ + n_defer_)
             **/
            slot_type slot_[];
        }; /*CallbackSet*/
    } /*namespace fn*/
} /*namespace xo*/

/* end CallbackSet.hpp */
