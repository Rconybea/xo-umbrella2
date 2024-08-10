/* file expect_symbol_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_symbol_xs.hpp"

namespace xo {
    namespace scm {
        std::unique_ptr<expect_symbol_xs>
        expect_symbol_xs::expect_symbol_expression() {
            return std::make_unique<expect_symbol_xs>(expect_symbol_xs());
        }

        expect_symbol_xs::expect_symbol_xs()
            : exprstate(exprstatetype::expect_symbol)
        {}

    } /*namespace scm*/
} /*namespace xo*/


/* end expect_symbol_xs.cpp */
