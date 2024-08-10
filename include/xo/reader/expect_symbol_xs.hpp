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

            static std::unique_ptr<expect_symbol_xs> expect_symbol_expression();
        };
    } /*namespace scm*/
} /*namespace xo*/


/* end expect_symbol_xs.hpp */
