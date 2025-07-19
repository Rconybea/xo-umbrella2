/* file exprstatestack.cpp
 *
 * author: Roland Conybeare
 */

#include "exprstatestack.hpp"
#include <cstdint>

namespace xo {
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
        exprstatestack::push_exprstate(std::unique_ptr<exprstate> exs) {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag),
                      xtag("exs", exs.get()));

            std::size_t z = stack_.size();

            stack_.resize(z+1);

            stack_[z] = std::move(exs);
        }

        std::unique_ptr<exprstate>
        exprstatestack::pop_exprstate() {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag),
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
        exprstatestack::print(std::ostream & os) const {
            os << "<exprstatestack"
               << xtag("size", stack_.size())
               << std::endl;

            for (std::size_t i = 0, z = stack_.size(); i < z; ++i) {
                os << "  [" << z-i-1 << "] "
                   << stack_[i].get()
                   << std::endl;
            }

            os << ">" << std::endl;
        }

        bool
        exprstatestack::pretty_print(const ppindentinfo & ppii) const
        {
            ppstate * pps = ppii.pps();

            if (ppii.upto()) {
                if (stack_.size() > 1)
                    return false;

                if (!pps->print_upto("<exprstatestack"))
                    return false;

                if (!pps->print_upto_tag("size", stack_.size()))
                    return false;

                /** always multiple lines if more than one element in stack **/
                if ((stack_.size() > 0)
                    && !pps->print_upto_tag("[0]", *stack_[0].get()))
                {
                    return false;
                }

                pps->write(">");

                return true;
            } else {
                pps->write("<exprstatestack");

                pps->newline_pretty_tag(ppii.ci1(), "size", stack_.size());

                for (std::size_t i = 0, z = stack_.size(); i < z; ++i) {
                    std::string i_str = tostr("[", z-i-1, "]");

                    pps->newline_pretty_tag(ppii.ci1(), i_str, *stack_[i].get());
                }

                pps->write(">");

                return false;
            }
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end exprstatestack.cpp */
