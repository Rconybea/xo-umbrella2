/* @file UpCallbackSet.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "CallbackSetImpl.hpp"
#include "callback_concept.hpp"
#include <memory>

namespace xo {
    namespace fn {
        template <typename NativeFn>
        using UpCallbackSet = CallbackSetImpl<std::unique_ptr<NativeFn>>;

        template <typename NativeFn, typename MemberFn>
        requires(callback_concept<NativeFn>)
        class UpNotifyCallbackSet : public UpCallbackSet<NativeFn> {
        public:
            explicit UpNotifyCallbackSet(MemberFn fn)
                : privileged_member_fn_{fn} {}

            template <typename... Tn>
            void operator()(Tn&&... args) {
                this->invoke(this->privileged_member_fn_, args...);
            }

        private:
            /** implements UpNotifyCallbackSet's call operator **/
            MemberFn privileged_member_fn_;
        };
    } /*namespace fn*/
} /*namespace xo*/

/* end UpCallbackSet.hpp */
