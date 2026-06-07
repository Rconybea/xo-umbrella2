/* @file callback_concept.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

namespace xo {
    namespace fn {
        /** @concept callback_concept
         *  @brief Concept for functors that can be used with @ref CallbackSetImpl
         *
         **/
        template <typename Callback>
        concept callback_concept = requires(Callback cb)
        {
            // no constraints on how callback is invoked.

            { cb.notify_add_callback() };
            { cb.notify_remove_callback() };
        };

    }
}

/* callback_concept.hpp */
