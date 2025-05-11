/* file expect_type_xs.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "exprstate.hpp"

namespace xo {
    namespace scm {
        /** @class expect_type_xs
         *  @brief state-machine for accepting a typename-expression
         **/
        class expect_type_xs : public exprstate {
        public:
            expect_type_xs();

            static void start(parserstatemachine * p_stack);

            virtual void on_symbol_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;

        private:
            static std::unique_ptr<expect_type_xs> make();
        };
    } /*namespace scm*/
} /*namespace xo*/


/* end expect_type_xs.hpp */
