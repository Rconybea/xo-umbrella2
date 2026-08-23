/* file exprstatestack.cpp
 *
 * author: Roland Conybeare
 */

#include "exprstatestack.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/concat.hpp>
#include <cstdint>

namespace xo {
    using xo::pp::scope;
    using xo::pp::field;
    using xo::pp::xtag;
    using xo::pp::concat;

    namespace scm {
        exprstate &
        exprstatestack::top_exprstate() {
            std::size_t z = stack_.size();

            if (z == 0) {
                throw std::runtime_error
                    ("parser::top_exprstate: unexpected empty stack");
            }

            return *(stack_[z-1]);
        }

        void
        exprstatestack::reset_to_toplevel() {
            this->stack_.resize(1);
        }

        void
        exprstatestack::push_exprstate(std::unique_ptr<exprstate> exs) {
            constexpr bool c_debug_flag = false;
            scope log(XO_DEBUG_(c_debug_flag),
                      xtag("exs", exs.get()));

            std::size_t z = stack_.size();

            stack_.resize(z+1);

            stack_[z] = std::move(exs);
        }

        std::unique_ptr<exprstate>
        exprstatestack::pop_exprstate() {
            constexpr bool c_debug_flag = false;
            scope log(XO_DEBUG_(c_debug_flag),
                      xtag("top.exstype", top_exprstate().exs_type()));

            std::size_t z = stack_.size();

            if (z > 0) {
                std::unique_ptr<exprstate> top = std::move(stack_[z-1]);

                stack_.resize(z-1);

                return top;
            } else {
                return nullptr;
            }
        }

        void
        exprstatestack::pretty(xo::pp::PpSink & sink) const
        {
            /* force_break preserves the legacy policy, which bailed out of the
             * fit pass ("always multiple lines if more than one element in
             * stack") rather than letting the margin decide.
             */
            const std::size_t z = stack_.size();

            auto st = sink.struct_open("exprstatestack", z > 1 /*force_break*/);

            st.field("size", z);

            for (std::size_t i = 0; i < z; ++i)
                st.field(concat("[", z-i-1, "]"), stack_[i].get());
        }
    } /*namespace scm*/

    namespace pp {
        XO_PRETTIFIER_VIA_PRETTY_METHOD(xo::scm::exprstatestack)
    } /*namespace pp*/
} /*namespace xo*/

/* end exprstatestack.cpp */
