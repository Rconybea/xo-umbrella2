/* @file CallbackSet.hpp */

#pragma once

//#include "indentlog/scope.hpp"
//#include "indentlog/print/tag.hpp"
#include <functional>
#include <vector>
#include <cstdint>

namespace xo {
    namespace fn {
        /* identifies a particular callback in a CallbackSet (see below).
         * an unique id is created:
         *   CallbackSetImpl cbset = ...;
         *   CallbackId cb_id = cbset.add_callback(..);
         *
         * can use id to remove callback later:
         *   cbset.remove_callback(cb_id);
         *
         * Tag so xo-callback can be header-only
         */
        template <typename Tag>
        class CallbackIdImpl {
        public:
            CallbackIdImpl() = default;
            explicit CallbackIdImpl(uint32_t id) : id_{id} {}

            /* generate a globally-unique id (not threadsafe) */
            static CallbackIdImpl generate() {
                static CallbackIdImpl s_last_id;

                s_last_id = CallbackIdImpl(s_last_id.id() + 1);

                return s_last_id;
            } /*generate*/

            uint32_t id() const { return id_; }

        private:
            uint32_t id_ = 0;
        }; /*CallbackIdImpl*/

        template <typename Tag>
        inline bool operator==(CallbackIdImpl<Tag> lhs, CallbackIdImpl<Tag> rhs) { return lhs.id() == rhs.id(); }
        template <typename Tag>
        inline bool operator!=(CallbackIdImpl<Tag> lhs, CallbackIdImpl<Tag> rhs) { return lhs.id() != rhs.id(); }

        using CallbackId = CallbackIdImpl<class CallbackId_tag>;

        /*   queue add/remove callback instructions encountered during callback
         *   execution, to avoid invalidating vector iterator.
         *
         */
        template<typename Fn>
        struct ReentrantCbsetCmd {
            enum CbsetCmdEnum { AddCallback, RemoveCallback };

            ReentrantCbsetCmd() = default;
            ReentrantCbsetCmd(CbsetCmdEnum cmd, CallbackId id, Fn const & fn)
                : cmd_{cmd}, id_{id}, fn_{fn} {}

            static ReentrantCbsetCmd add(CallbackId id, Fn const & fn) {
                return ReentrantCbsetCmd{AddCallback, id, fn};
            } /*add*/

            static ReentrantCbsetCmd remove(CallbackId id) {
                return ReentrantCbsetCmd{RemoveCallback, id, Fn()};
            } /*remove*/

            bool is_add() const { return cmd_ == AddCallback; }
            bool is_remove() const { return cmd_ == RemoveCallback; }
            CallbackId id() const { return id_; }
            Fn const & fn() const { return fn_; }

        private:
            /* AddCallback:    deferred CallbackSet<Fn>::add_callback(.fn)
             * RemoveCallback: deferred CallbackSet<Fn>::remove_callback(.fn)
             */
            CbsetCmdEnum cmd_ = AddCallback;
            /* operate on callback with this id */
            CallbackId id_;
            /* callback function to add/remove */
            Fn fn_;
        }; /*ReentrantCbsetCmd*/

        /* record for remembering a single callback.
         * callbacks are given unique ids so they can be removed later
         */
        template<typename Fn>
        struct CbRecd {
            CbRecd(CallbackId id, Fn const & fn) : id_{id}, fn_{fn} {}

            CallbackId id_;
            Fn fn_;
        }; /*CbRecd*/

        /* If Fnptr is a type such that this works:
         *   Fnptr fn = ...;
         *   using Fn = Fnptr::element_type;
         *   Fn * native_fn = fn.get();
         *   (native_fn->*member_fn)(args ...);
         *
         * then
         *   CallbackSet<Fnptr> cbset = ...;
         *   cbset.invoke(&Fn::member_fn, args...)
         *
         * calls
         *   (cb->*member_fn)(args...)
         *
         * for each callback cb in this set.
         *
         * In addition,  calls hook methods:
         *   cb->notify_add_callback()
         *   cb->notify_remove_callback()
         * when adding/removing callback.
         *
         * Require:
         * - Fnptr::element_type
         * - Fnptr::get() -> Fnptr::element_type const *
         * - can invoke (Fnptr->*member_fn)(...)
         *
         * implementation is reentrant: running callbacks can safely make
         * add/remove calls on the cbset that invoked them.
         *
         * not threadsafe.
         */
        template<typename Fn>
        class CallbackSetImpl {
        public:
            using callback_type = typename Fn::element_type;
            //using scope = xo::scope;

        public:
            CallbackSetImpl() = default;

            /* support for range iterators */
            typename std::vector<CbRecd<Fn>>::const_iterator begin() const { return cb_v_.begin(); }
            typename std::vector<CbRecd<Fn>>::const_iterator end() const { return cb_v_.end(); }

            /* invoke callbacks registered with this callback set */
            template<typename ... Tn, typename ... Sn>
            void invoke(void (callback_type::* member_fn)(Sn... args), Tn&&... args) {
                this->cb_running_ = true;

                try {
                    for(CbRecd<Fn> const & cb_recd : this->cb_v_) {
                        callback_type * native_cb = cb_recd.fn_.get();

                        /* clang11 doesn't like (with cb=cb_recd.fn_)
                         *   cb->*member_fn
                         * when cb-> is overloaded
                         */
                        (native_cb->*member_fn)(args...);
                    }

                    this->make_deferred_changes();
                } catch(...) {
                    this->make_deferred_changes();
                    throw;
                }
            } /*operator()*/

            /* call fn(cb) for each callback present in this set */
            void visit_callbacks(std::function<void (Fn const &)> fn) const {
                CallbackSetImpl * self = const_cast<CallbackSetImpl *>(this);

                self->cb_running_ = true;

                try {
                    for(Fn const & cb : this->cb_v_)
                        fn(cb);

                    this->make_deferred_changes();
                } catch(...) {
                    this->make_deferred_changes();
                    throw;
                }
            } /*visit_callbacks*/

            /* add callback target_fn to this callback set.
             * reentrant
             */
            CallbackId add_callback(Fn const & target_fn) {
                CallbackId id = CallbackId::generate();

                if(this->cb_running_) {
                    /* defer until callback execution completes */
                    this->reentrant_cmd_v_.push_back(ReentrantCbsetCmd<Fn>::add(id, target_fn));
                } else {
#ifdef NOT_USING
                    constexpr bool c_debug_enabled_flag = false;
                    scope lscope(reflect::type_name<CallbackSetImpl>(),
                                 "::add_callback", c_debug_enabled_flag);

                    if (c_debug_enabled_flag) {
                        lscope.log("before appending .cb_v[]",
                                   xo::xtag("target_fn", (void*)target_fn.get()),
                                   xo::xtag("target_fn.refcnt",
                                            target_fn->reference_counter()));
                    }
#endif

                    this->cb_v_.push_back(CbRecd(id, target_fn));

#ifdef NOT_USING
                    if (c_debug_enabled_flag) {
                        lscope.log("after appending .cb_v[]",
                                   xo::xtag("target_fn", (void *)target_fn.get()),
                                   xo::xtag("target_fn.refcnt",
                                            target_fn->reference_counter()));
                    }
#endif
                }

                return id;
            } /*add_callback*/

            void remove_callback(CallbackId id) {
                if(this->cb_running_) {
                    /* defer until callback execution completes */
                    this->reentrant_cmd_v_.push_back(ReentrantCbsetCmd<Fn>::remove(id));
                } else {
                    this->remove_callback_impl(id);
                }

            } /*remove_callback*/

#ifdef NOT_USING
            /* remove callback target_fn from this callback set.
             * noop if callback is not present
             */
            void remove_callback(Fn const & target_fn) {
                if(this->cb_running_) {
                    /* defer until callback execution completes */
                    this->reentrant_cmd_v_.push_back(ReentrantCbsetCmd<Fn>::remove(target_fn));
                } else {
                    this->remove_callback_impl(target_fn);
                }
            } /*remove_callback*/
#endif

        private:
            /* apply deferred changes to .cb_v[] */
            void make_deferred_changes() {
                this->cb_running_ = false;

                std::vector<ReentrantCbsetCmd<Fn>> cmd_v;
                std::swap(cmd_v, this->reentrant_cmd_v_);

                for(ReentrantCbsetCmd<Fn> const & cmd : cmd_v) {
                    if(cmd.is_add()) {
                        this->cb_v_.push_back(CbRecd(cmd.id(), cmd.fn()));

                        cmd.fn()->notify_add_callback();
                    } else if(cmd.is_remove()) {
                        this->remove_callback_impl(cmd.id());
                    }
                }
            } /*make_deferred_changes*/

            void remove_callback_impl(CallbackId target_id) {
                for (auto ix = this->cb_v_.begin(); ix != this->cb_v_.end(); ++ix) {
                    if (ix->id_ == target_id) {
                        Fn target_fn = ix->fn_;

                        this->cb_v_.erase(ix);

                        target_fn->notify_remove_callback();
                        break;
                    }
                }
            } /*remove_callback_impl*/

        private:
            bool cb_running_ = false;
            /* collection of callback functions */
            std::vector<CbRecd<Fn>> cb_v_;
            /* when a callback registered with *this, while running,
             * attempts to add/remove a callback to/from this set
             * (including removing itself),
             * must defer until all callbacks have executed.
             * remember deferred instructions here.
             */
            std::vector<ReentrantCbsetCmd<Fn>> reentrant_cmd_v_;
        }; /*CallbackSetImpl*/

        template<typename NativeFn>
        using RpCallbackSet = CallbackSetImpl<xo::ref::rp<NativeFn>>;

        /* like RpCallbackSet<NativeFn>,
         * but also provides overload(s) for operator()(..)
         */
        template<typename NativeFn, typename MemberFn>
        class NotifyCallbackSet : public RpCallbackSet<NativeFn> {
        public:
            NotifyCallbackSet(MemberFn fn)
                : privileged_member_fn_{fn} {}

            template<typename ... Tn>
            void operator()(Tn&&... args) {
                this->invoke(this->privileged_member_fn_, args...);
            } /*operator()*/

        private:
            /* implements NotifyCallbackSet's operator()(...) */
            MemberFn privileged_member_fn_;
        }; /*NotifyCallbackSet*/

        template<typename NativeFn, typename Sret, typename ... Sn>
        inline NotifyCallbackSet<NativeFn, Sret (NativeFn::*)(Sn...)>
        make_notify_cbset(Sret (NativeFn::* member_fn)(Sn...)) {
            return NotifyCallbackSet<NativeFn, Sret (NativeFn::*)(Sn...)>(member_fn);
        } /*make_notify_cbset*/
    } /*namespace fn*/
} /*namespace xo*/

/* end CallbackSet.hpp */
