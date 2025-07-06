/* file envframestack.cpp
 *
 * author: Roland Conybeare
 */

#include "envframestack.hpp"

namespace xo {
    using xo::ast::LocalEnv;
    using xo::ast::Variable;

    namespace scm {
        bp<LocalEnv>
        envframestack::top_envframe() {
            std::size_t z = stack_.size();

            if (z == 0) {
                throw std::runtime_error
                    ("parser::top_exprstate: unexpected empty stack");
            }

            return stack_[z-1].get();
        }

        void
        envframestack::push_envframe(const rp<LocalEnv> & frame)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag),
                      xtag("frame", frame));

            std::size_t z = stack_.size();

            stack_.resize(z+1);

            stack_[z] = frame;
        }

        rp<LocalEnv>
        envframestack::pop_envframe() {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            std::size_t z = stack_.size();

            if (z > 0) {
                //std::unique_ptr<exprstate> top = std::move(stack_[z-1]);

                rp<LocalEnv> retval = stack_.at(z-1);

                stack_.resize(z-1);

                return retval;
            } else {
                return nullptr;
            }
        }

        bp<Variable>
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
        envframestack::print(std::ostream & os) const {
            os << "<envframestack"
               << xtag("size", stack_.size())
               << std::endl;

            for (std::size_t i = 0, z = stack_.size(); i < z; ++i) {
                os << "  [" << z-i-1 << "] "
                   << stack_[i]
                   << std::endl;
            }

            os << ">" << std::endl;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end envframestack.cpp */
