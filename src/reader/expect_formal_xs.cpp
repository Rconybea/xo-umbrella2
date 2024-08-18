/* file expect_formal_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_formal_xs.hpp"
#include "xo/expression/Variable.hpp"

namespace xo {
    using xo::ast::Variable;
    using xo::reflect::TypeDescr;

    namespace scm{
        const char *
        formalstatetype_descr(formalstatetype x) {
            switch (x) {
            case formalstatetype::invalid:
            case formalstatetype::n_formalstatetype:
                return "?formalstatetype";
            case formalstatetype::formal_0:
                return "formal_0";
            case formalstatetype::formal_1:
                return "formal_1";
            case formalstatetype::formal_2:
                return "formal_2";
            }

            return "???formalstatetype";
        }

        std::unique_ptr<expect_formal_xs>
        expect_formal_xs::make() {
            return std::make_unique<expect_formal_xs>(expect_formal_xs());
        }

        expect_formal_xs::expect_formal_xs()
            : exprstate(exprstatetype::expect_formal)
        {}

        void
        expect_formal_xs::on_symbol(const std::string & symbol_name,
                                    exprstatestack * p_stack,
                                    rp<Expression> * p_emit_expr)
        {
            if (this->formalxs_type_ == formalstatetype::formal_0) {
                this->formalxs_type_ = formalstatetype::formal_1;
                this->result_.assign_name(symbol_name);
            } else {
                exprstate::on_symbol(symbol_name,
                                     p_stack,
                                     p_emit_expr);
            }
        }

        void
        expect_formal_xs::on_colon_token(const token_type & tk,
                                         exprstatestack * p_stack
                                         /* rp<Expression> * p_emit_expr */)
        {
            if (this->formalxs_type_ == formalstatetype::formal_1) {
                this->formalxs_type_ = formalstatetype::formal_2;
            } else {
                exprstate::on_colon_token(tk,
                                          p_stack);
            }
        }

        void
        expect_formal_xs::on_typedescr(TypeDescr td,
                                       exprstatestack * p_stack,
                                       rp<Expression> * p_emit_expr)
        {
            if (this->formalxs_type_ == formalstatetype::formal_2) {
                this->result_.assign_td(td);

                std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

                rp<Variable> var = Variable::make(result_.name(),
                                                  result_.td());

                p_stack->top_exprstate().on_formal(var, p_stack, p_emit_expr);
            } else {
                exprstate::on_typedescr(td, p_stack, p_emit_expr);
            }
        }

        void
        expect_formal_xs::print(std::ostream & os) const {
            os << "<expect_formal_xs"
               << xtag("type", formalxs_type_);
            if (!result_.name().empty())
                os << xtag("result.name", result_.name());
            if (result_.td())
                os << xtag("result.td", result_.td());
            os << ">";
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end expect_formal_xs.cpp */
