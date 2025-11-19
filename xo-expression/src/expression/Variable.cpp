/* @file Variable.cpp */

#include "Variable.hpp"
#include "SymbolTable.hpp"
#include "pretty_expression.hpp"

namespace xo {
    namespace scm {
        std::string
        Variable::gensym(const std::string & prefix) {
            static std::size_t s_counter = 0;

            ++s_counter;

            char buf[32];
            snprintf(buf, sizeof(buf), "%ld", s_counter);

            return prefix + std::string(buf);
        }

        void
        Variable::attach_envs(bp<SymbolTable> e) {
            /** e makes accessible all enclosing lexical scopes **/
            if (this->path_.i_link_ == -2 /*sentinel*/) {
                this->path_ = e->lookup_binding(this->name_);
            } else {
                /* have already established binding for this Variable */
            }
        } /*attach_envs*/

        void
        Variable::display(std::ostream & os) const {
            os << "<Variable"
               << xtag("name", name_);
            if (this->valuetype())
                os << xtag("type", this->valuetype()->short_name());
            else
                os << xtag("type", "nullptr");
            os << ">";
        } /*display*/

        std::uint32_t
        Variable::pretty_print(const ppindentinfo & ppii) const {
            /* 1. rtag instead of refrtag:
             *    print::quot() is a temporary rvalue; lifetime ends before control enters pretty_struct()
             */
            return ppii.pps()->pretty_struct(ppii, "Variable",
                                             refrtag("name", name_),
                                             rtag("type", print::unq(this->valuetype()
                                                                      ? this->valuetype()->short_name()
                                                                      : "nullptr")));
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end Variable.cpp */
