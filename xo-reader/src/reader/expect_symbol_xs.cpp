/* file expect_symbol_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_symbol_xs.hpp"
#include "exprstatestack.hpp"
#include "parserstatemachine.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>

namespace xo {
    using xo::pp::xtag;
    using xo::pp::scope;
    namespace scm {
        std::unique_ptr<expect_symbol_xs>
        expect_symbol_xs::make() {
            return std::make_unique<expect_symbol_xs>(expect_symbol_xs());
        }

        void
        expect_symbol_xs::start(parserstatemachine * p_psm)
        {
            p_psm->push_exprstate(expect_symbol_xs::make());
        }

        expect_symbol_xs::expect_symbol_xs()
            : exprstate(exprstatetype::expect_symbol)
        {}

        void
        expect_symbol_xs::on_symbol_token(const token_type & tk,
                                          parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG_(c_debug_flag));

            log && log(xtag("tk", tk));

            assert(&p_psm->top_exprstate() == this);

            /* have to do pop first, before sending symbol to
             * the o.g. symbol-requester
             */
            std::unique_ptr<exprstate> self{p_psm->pop_exprstate()};

            p_psm->on_symbol(tk.text());
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end expect_symbol_xs.cpp */
