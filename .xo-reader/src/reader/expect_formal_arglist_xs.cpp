/* file expect_formal_arglist_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_formal_arglist_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
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
        expect_formal_arglist_xs::start(parserstatemachine * p_psm)
        {
            p_psm->push_exprstate(expect_formal_arglist_xs::make());
        }

        const char *
        expect_formal_arglist_xs::get_expect_str() const {
            switch (farglxs_type_) {
            case formalarglstatetype::invalid:
            case formalarglstatetype::n_formalarglstatetype:
                assert(false); // impossible
                break;
            case formalarglstatetype::argl_0:
                return "leftparen";
            case formalarglstatetype::argl_1a:
                return "formal-name";
            case formalarglstatetype::argl_1b:
                return "comma|rightparen";
            }

            return "?expect";
        }

        expect_formal_arglist_xs::expect_formal_arglist_xs()
            : exprstate(exprstatetype::expect_formal_arglist),
              farglxs_type_{formalarglstatetype::argl_0}
        {}

        void
        expect_formal_arglist_xs::on_leftparen_token(const token_type & tk,
                                                     parserstatemachine * p_psm)
        {
            if (farglxs_type_ == formalarglstatetype::argl_0) {
                this->farglxs_type_ = formalarglstatetype::argl_1a;
                /* TODO: refactor to have setup method on each exprstate */
                expect_formal_xs::start(p_psm);
            } else {
                exprstate::on_leftparen_token(tk, p_psm);
            }
        }

        void
        expect_formal_arglist_xs::on_formal(const rp<Variable> & formal,
                                            parserstatemachine * p_psm)
        {
            if (farglxs_type_ == formalarglstatetype::argl_1a) {
                this->farglxs_type_ = formalarglstatetype::argl_1b;
                this->argl_.push_back(formal);
            } else {
                exprstate::on_formal(formal, p_psm);
            }
        }

        void
        expect_formal_arglist_xs::on_comma_token(const token_type & tk,
                                                 parserstatemachine * p_psm)
        {
            if (farglxs_type_ == formalarglstatetype::argl_1b) {
                this->farglxs_type_ = formalarglstatetype::argl_1a;
                expect_formal_xs::start(p_psm);
            } else {
                exprstate::on_comma_token(tk, p_psm);
            }
        }

        void
        expect_formal_arglist_xs::on_rightparen_token(const token_type & tk,
                                                      parserstatemachine * p_psm)
        {
            if (farglxs_type_ == formalarglstatetype::argl_1b) {
                std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                p_psm->top_exprstate().on_formal_arglist(this->argl_, p_psm);
            } else {
                exprstate::on_rightparen_token(tk, p_psm);
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
