/* file expect_formal_arglist_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_formal_arglist_xs.hpp"

namespace xo {
    namespace scm {
        std::unique_ptr<expect_formal_arglist_xs>
        expect_formal_arglist_xs::make() {
            return std::make_unique<expect_formal_arglist_xs>
                (expect_formal_arglist_xs());
        }

        expect_formal_arglist_xs::expect_formal_arglist_xs()
            : farglxs_type_{formalarglstatetype::argl_0}
        {}

        void
        expect_formal_arglist_xs::on_leftparen_token(const token_type & tk,
                                                     exprstatestack * p_stack,
                                                     rp<Expression> * p_emit_expr)
        {
            if (farglxs_type_ == formalarglstatetype::argl_0) {
                this->farglxs_type_ = formalarglstatetype::argl_1;
                return;
            }

            exprstate::on_leftparen_token(tk, p_stack, p_emit_expr);
        }

    } /*namespace scm*/
} /*namespace xo*/


/* end expect_formal_arglist_xs.cpp */
