/* file expect_type_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_type_xs.hpp"
#include "exprstate.hpp"

namespace xo {
    namespace scm {
        std::unique_ptr<expect_type_xs>
        expect_type_xs::make() {
            return std::make_unique<expect_type_xs>(expect_type_xs());
        }

        expect_type_xs::expect_type_xs()
            : exprstate(exprstatetype::expect_type)
        {}

    } /*namespace scm*/
} /*namespace xo*/


/* end expect_type_xs.cpp */
