/* file expect_formal_arglist_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_formal_arglist_xs.hpp"
#include "expect_formal_xs.hpp"
#include "expect_symbol_xs.hpp"
#include "xo/expression/Variable.hpp"
#include "xo/indentlog/print/vector.hpp"

namespace xo {
    namespace scm {
        const char *
        formalarglstatetype_descr(formalarglstatetype x) {
            switch (x) {
            case formalarglstatetype::invalid:
                return "invalid";
            case formalarglstatetype::argl_0:
                return "argl_0";
            case formalarglstatetype::argl_1a:
                return "argl_1a";
            case formalarglstatetype::argl_1b:
                return "argl_1b";
            case formalarglstatetype::n_formalarglstatetype:
                break;
            }

            return "?formalarglstatetype";
        }

        std::unique_ptr<expect_formal_arglist_xs>
        expect_formal_arglist_xs::make() {
            return std::make_unique<expect_formal_arglist_xs>
                (expect_formal_arglist_xs());
        }

        void
        expect_formal_arglist_xs::start(exprstatestack * p_stack)
        {
            p_stack->push_exprstate(expect_formal_arglist_xs::make());
        }

        expect_formal_arglist_xs::expect_formal_arglist_xs()
            : exprstate(exprstatetype::expect_formal_arglist),
              farglxs_type_{formalarglstatetype::argl_0}
        {}

        void
        expect_formal_arglist_xs::on_leftparen_token(const token_type & tk,
                                                     exprstatestack * p_stack,
                                                     rp<Expression> * p_emit_expr)
        {
            if (farglxs_type_ == formalarglstatetype::argl_0) {
                this->farglxs_type_ = formalarglstatetype::argl_1a;
                /* TODO: refactor to have setup method on each exprstate */
                p_stack->push_exprstate(expect_formal_xs::make());
                expect_symbol_xs::start(p_stack);
            } else {
                exprstate::on_leftparen_token(tk, p_stack, p_emit_expr);
            }
        }

        void
        expect_formal_arglist_xs::on_formal(const rp<Variable> & formal,
                                            exprstatestack * p_stack,
                                            rp<Expression> * p_emit_expr)
        {
            if (farglxs_type_ == formalarglstatetype::argl_1a) {
                this->farglxs_type_ = formalarglstatetype::argl_1b;
                this->argl_.push_back(formal);
            } else {
                exprstate::on_formal(formal, p_stack, p_emit_expr);
            }
        }

        void
        expect_formal_arglist_xs::on_comma_token(const token_type & tk,
                                                 exprstatestack * p_stack,
                                                 rp<Expression> * p_emit_expr)
        {
            if (farglxs_type_ == formalarglstatetype::argl_1b) {
                this->farglxs_type_ = formalarglstatetype::argl_1a;
                p_stack->push_exprstate(expect_formal_xs::make());
            } else {
                exprstate::on_comma_token(tk, p_stack, p_emit_expr);
            }
        }

        void
        expect_formal_arglist_xs::on_rightparen_token(const token_type & tk,
                                                      exprstatestack * p_stack,
                                                      rp<Expression> * p_emit_expr)
        {
            if (farglxs_type_ == formalarglstatetype::argl_1b) {
                std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

                p_stack->top_exprstate().on_formal_arglist(this->argl_,
                                                           p_stack, p_emit_expr);
            } else {
                exprstate::on_rightparen_token(tk, p_stack, p_emit_expr);
            }
        }

        void
        expect_formal_arglist_xs::print(std::ostream & os) const {
            os << "<expect_formal_arglist_xs"
               << xtag("type", farglxs_type_);
            os << xtag("farglxs_type", farglxs_type_);
            os << xtag("argl", argl_);
            os << ">";
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end expect_formal_arglist_xs.cpp */
