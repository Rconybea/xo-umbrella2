/* file envframestack.cpp
 *
 * author: Roland Conybeare
 */

#include "envframestack.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/concat.hpp>

namespace xo {
    using xo::pp::field;
    using xo::pp::xtag;
    using xo::pp::scope;
    using xo::scm::LocalSymtab;
    using xo::scm::Variable;

    namespace scm {
        bp<SymbolTable>
        envframestack::top_envframe() const {
            std::size_t z = stack_.size();

            if (z == 0) {
                throw std::runtime_error
                    ("parser::top_exprstate: unexpected empty stack");
            }

            return stack_[z-1].get();
        }

        void
        envframestack::push_envframe(const rp<SymbolTable> & frame)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG_(c_debug_flag),
                      xtag("frame", frame));

            std::size_t z = stack_.size();

            stack_.resize(z+1);

            stack_[z] = frame;
        }

        rp<SymbolTable>
        envframestack::pop_envframe() {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG_(c_debug_flag));

            std::size_t z = stack_.size();

            if (z > 0) {
                //std::unique_ptr<exprstate> top = std::move(stack_[z-1]);

                rp<SymbolTable> retval = stack_.at(z-1);

                stack_.resize(z-1);

                return retval;
            } else {
                return nullptr;
            }
        }

        bp<Expression>
        envframestack::lookup(const std::string & x) const {
            for (std::size_t i = 0, z = this->size(); i < z; ++i) {
                const auto & frame = (*this)[i];

                auto retval = frame->lookup_local(x);

                if (retval)
                    return retval;
            }

            return bp<Variable>::from_native(nullptr);
        }

        void
        envframestack::upsert(bp<Variable> x) {
            /* upsert should always happen in the innermost lexical context.
             * We are providing new variable binding (perhaps shadowing an existing binding)
             */
            this->top_envframe()->upsert_local(x);
        }

        void
        envframestack::pretty(xo::pp::PpSink & sink) const
        {
            /* force_break preserves the legacy policy, which bailed out of the
             * fit pass ("always multiple lines if more than one element in
             * stack") rather than letting the margin decide.
             */
            const std::size_t z = stack_.size();

            auto st = sink.struct_open("envframestack", z > 1 /*force_break*/);

            st.field("size", z);

            for (std::size_t i = 0; i < z; ++i)
                st.field(xo::pp::concat("[", z-i-1, "]"), stack_[i]);
        }
    } /*namespace scm*/

    namespace pp {
        XO_PRETTIFIER_VIA_PRETTY_METHOD(xo::scm::envframestack)
    }
} /*namespace xo*/

/* end envframestack.cpp */
