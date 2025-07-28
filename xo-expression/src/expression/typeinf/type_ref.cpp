/** @file type_ref.cpp **/

#include "typeinf/type_ref.hpp"

namespace xo {
    namespace scm {

        type_ref::type_ref(const type_var& id, TypeDescr td)
            : id_{id}, td_{td}
        {}

        bool type_ref::is_concrete() const { return td_ != nullptr; }

        type_ref
        type_ref::dwim(prefix_type prefix, TypeDescr td)
        {
            if (td) {
                /** type resolved, type variable not needed **/
                return type_ref(type_var(), td);
            } else {
                /** type not resolved, assign a unique type variable **/
                return type_ref(generate_unique(prefix), td);
            }
        }

        auto
        type_ref::generate_unique(xo::scm::prefix_type prefix) -> xo::scm::type_var
        {
            static uint32_t s_counter = 0;

            s_counter = (s_counter + 1) % 100000000;

            char buf [type_var::fixed_capacity];
            int n = snprintf(buf, sizeof(buf), "%s:%u", prefix.c_str(), s_counter);

            assert(n < static_cast<int>(type_var::fixed_capacity));

            // not necessary, but remove all doubt
            // max:
            //  7 chars for prefix
            //  8 chars for u32 % 1000000000
            //
            buf [type_var::fixed_capacity - 1] = '\0';

            return buf;
        }

        void
        type_ref::resolve_to(TypeDescr td)
        {
            assert(!td_);

            this->td_ = td;
        }

    } /*namespace scm*/
} /*namespace xo*/


/** end type_ref.cpp **/
