/** @file sequence_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"

namespace xo {
    namespace scm {
        class sequence_xs : public exprstate {
        public:
            sequence_xs();

            static void start(parserstatemachine * p_psm);

            virtual void on_expr(ref::brw<Expression> expr,
                                 parserstatemachine * p_psm) override;

            virtual void on_rightbrace_token(const token_type & tk,
                                             parserstatemachine * p_psm) override;

        private:
            static std::unique_ptr<sequence_xs> make();

        private:
            std::vector<rp<Expression>> expr_v_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end sequence_xs.hpp **/
