/* @file Refcounted.cpp */

#include "Refcounted.hpp"

namespace xo {
    namespace ref {
#ifdef XO_INTRUSIVE_PTR_ENABLE_LOGGING
        namespace {
            /* verbose logging for intrusive_ptr */
            static bool s_logging_enabled = false;

            void
            intrusive_ptr_log_aux(std::string_view const & self_type,
                                  std::string_view const & method_name,
                                  void * this_ptr,
                                  Refcount * x)
            {
                scope lscope(XO_LITERAL(log_level::verbose, self_type, method_name),
                             "enter",
                             xtag("this", this_ptr),
                             xtag("x", x),
                             xtag("n", intrusive_ptr_refcount(x)));
            } /*intrusive_ptr_log_aux*/
        } /*namespace*/

        void
        intrusive_ptr_set_debug(bool debug_flag) {
            s_logging_enabled = debug_flag;
        } /*intrusive_ptr_set_debug*/

        void
        intrusive_ptr_log_ctor(std::string_view const & self_type,
                               void * this_ptr,
                               Refcount * x)
        {
            if (s_logging_enabled)
                intrusive_ptr_log_aux(self_type, "::ctor", this_ptr, x);
        } /*intrusive_ptr_log_ctor*/

        void
        intrusive_ptr_log_actor(std::string_view const & self_type,
                                void * this_ptr,
                                Refcount * x)
        {
            if (s_logging_enabled)
                intrusive_ptr_log_aux(self_type, "::actor", this_ptr, x);
        } /*intrusive_ptr_log_actor*/

        void
        intrusive_ptr_log_cctor(std::string_view const & self_type,
                                void * this_ptr,
                                Refcount * x)
        {
            if (s_logging_enabled)
                intrusive_ptr_log_aux(self_type, "::cctor", this_ptr, x);
        } /*intrusive_ptr_log_cctor*/

        void
        intrusive_ptr_log_mctor(std::string_view const & self_type,
                                void * this_ptr,
                                Refcount * x)
        {
            if (s_logging_enabled)
                intrusive_ptr_log_aux(self_type, "::mctor", this_ptr, x);
        } /*intrusive_ptr_log_mctor*/

        void
        intrusive_ptr_log_dtor(std::string_view const & self_type,
                               void * this_ptr,
                               Refcount * x)
        {
            if (s_logging_enabled)
                intrusive_ptr_log_aux(self_type, "::dtor", this_ptr, x);
        } /*intrusive_ptr_log_dtor*/

        void
        intrusive_ptr_log_assign(std::string_view const & self_type,
                                 void * this_ptr,
                                 Refcount * x)
        {
            if (s_logging_enabled)
                intrusive_ptr_log_aux(self_type, "::=", this_ptr, x);
        } /*intrusive_ptr_log_assign*/

        void
        intrusive_ptr_log_massign(std::string_view const & self_type,
                                  void * this_ptr,
                                  Refcount * x)
        {
            if (s_logging_enabled)
                intrusive_ptr_log_aux(self_type, "::m=", this_ptr, x);
        } /*intrusive_ptr_log_massign*/
#endif

        void
        intrusive_ptr_add_ref(Refcount * x)
        {
            /* for adding reference -- can use relaxed order,
             * since any reordering of a set of intrusive_ptr_add_ref()
             * calls is ok,   provided no intervening intrusive_ptr_release()
             * calls.
             */
            bool success = (x == nullptr);

            while(!success) {
                uint32_t n = x->reference_counter_.load(std::memory_order_relaxed);

                success = x->reference_counter_.compare_exchange_strong(n, n+1,
                                                                        std::memory_order_relaxed);
            }
        } /*intrusive_ptr_add_ref*/

        void
        intrusive_ptr_release(Refcount * x)
        {
            using xo::scope;
            using xo::xtag;

            scope log(XO_ENTER0(verbose),
                      "enter",
                      xtag("x", x),
                      xtag("n", x ? x->reference_counter_.load() : 0));

            /* for decrement,  need acq_rel ordering */
            bool success = (x == nullptr);
            uint32_t n = 0;

            while(!success) {
                n = x->reference_counter_.load(std::memory_order_acquire);

                if(n == static_cast<uint32_t>(-1)) {
                    log && log("detected double-delete attempt",
                               xtag("x", x),
                               xtag("n", n));
                    assert(false);
                }

                success = x->reference_counter_.compare_exchange_strong(n, n-1,
                                                                        std::memory_order_acq_rel);
            }

            if(n == 1) {
                /* just deleted the last reference,  so recover the object */

                log && log("delete object with 0 refs");

                /* for good measure: replace refcount with -1,
                 * in hope of detecting a double-delete attempt
                 */
                x->reference_counter_.store(static_cast<uint32_t>(-1));

                delete x;
            }
        } /*intrusive_ptr_release*/

    } /*namespace ref*/
} /*namespace xo*/

/* end Refcounted.cpp */
