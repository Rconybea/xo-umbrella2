/* file envframestack.cpp
 *
 * author: Roland Conybeare
 */

#include "envframestack.hpp"

namespace xo {
    using xo::ast::Variable;

    namespace scm {
        envframe &
        envframestack::top_envframe() {
            std::size_t z = stack_.size();

            if (z == 0) {
                throw std::runtime_error
                    ("parser::top_exprstate: unexpected empty stack");
            }

            return stack_[z-1];
        }

        void
        envframestack::push_envframe(envframe frame) {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag),
                      xtag("frame", frame));

            std::size_t z = stack_.size();

            stack_.resize(z+1);

            stack_[z] = std::move(frame);
        }

        void
        envframestack::pop_envframe() {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            std::size_t z = stack_.size();

            if (z > 0) {
                //std::unique_ptr<exprstate> top = std::move(stack_[z-1]);

                stack_.resize(z-1);

                //return top;
            } else {
                //return nullptr;
            }
        }

        rp<Variable>
        envframestack::lookup(const std::string & x) const {
            for (std::size_t i = 0, z = this->size(); i < z; ++i) {
                const auto & frame = (*this)[i];

                auto retval = frame.lookup(x);

                if (retval)
                    return retval;
            }

            return nullptr;
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
