/* file expect_formal_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_formal_xs.hpp"
#include "expect_symbol_xs.hpp"
#include "expect_type_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
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

        void
        expect_formal_xs::start(parserstatemachine * p_psm) {
            p_psm->push_exprstate(expect_formal_xs::make());

            expect_symbol_xs::start(p_psm);
        }

        expect_formal_xs::expect_formal_xs()
            : exprstate(exprstatetype::expect_formal)
        {}

        void
        expect_formal_xs::on_symbol(const std::string & symbol_name,
                                    parserstatemachine * p_psm)
        {
            if (this->formalxs_type_ == formalstatetype::formal_0) {
                this->formalxs_type_ = formalstatetype::formal_1;
                this->result_.assign_name(symbol_name);
            } else {
                exprstate::on_symbol(symbol_name, p_psm);
            }
        }

        void
        expect_formal_xs::on_colon_token(const token_type & tk,
                                         parserstatemachine * p_psm)
        {
            auto p_stack = p_psm->p_stack_;

            if (this->formalxs_type_ == formalstatetype::formal_1) {
                this->formalxs_type_ = formalstatetype::formal_2;
                expect_type_xs::start(p_stack);
                /* control reenters via expect_formal_xs::on_typedescr() */
            } else {
                exprstate::on_colon_token(tk,
                                          p_psm);
            }
        }

        void
        expect_formal_xs::on_typedescr(TypeDescr td,
                                       parserstatemachine * p_psm)
        {
            if (this->formalxs_type_ == formalstatetype::formal_2) {
                this->result_.assign_td(td);

                std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                rp<Variable> var = Variable::make(result_.name(),
                                                  result_.td());

                p_psm->top_exprstate().on_formal(var, p_psm);
            } else {
                exprstate::on_typedescr(td, p_psm);
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
