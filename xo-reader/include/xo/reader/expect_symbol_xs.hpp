/* file expect_symbol_xs.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "exprstate.hpp"

namespace xo {
    namespace scm {
        /** @class expect_symbol_xs
         *  @brief state machine to expect + capture a symbol
         *
         *  For example,  lhs in a define-expression
         **/
        class expect_symbol_xs : public exprstate {
        public:
            expect_symbol_xs();

            static std::unique_ptr<expect_symbol_xs> make();

            static void start(parserstatemachine * p_psm);

            virtual void on_symbol_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;
        };
    } /*namespace scm*/
} /*namespace xo*/


/* end expect_symbol_xs.hpp */
