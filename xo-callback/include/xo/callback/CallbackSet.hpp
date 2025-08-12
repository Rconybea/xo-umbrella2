// TOOD: rename to RpCallbackSet

/* @file CallbackSet.hpp */

#pragma once

#include "CallbackSetImpl.hpp"

namespace xo {
    namespace fn {
        template<typename NativeFn>
        using RpCallbackSet = CallbackSetImpl<rp<NativeFn>>;

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
            }

        private:
            /** implements NotifyCallbackSet's operator()(...) **/
            MemberFn privileged_member_fn_;
        };

        template<typename NativeFn, typename Sret, typename ... Sn>
        inline NotifyCallbackSet<NativeFn, Sret (NativeFn::*)(Sn...)>
        make_notify_cbset(Sret (NativeFn::* member_fn)(Sn...)) {
            return NotifyCallbackSet<NativeFn, Sret (NativeFn::*)(Sn...)>(member_fn);
        } /*make_notify_cbset*/

    } /*namespace fn*/
} /*namespace xo*/

/* end CallbackSet.hpp */
