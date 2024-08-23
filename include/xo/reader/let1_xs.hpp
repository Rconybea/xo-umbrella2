/* file let1_xs.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "exprstate.hpp"

namespace xo {
    namespace scm {
        class let1_xs : public exprstate {
        public:

            /** given local definition equivalent to
             *   def lhs_name = rhs
             *   rest...
             *  parse sequence of incoming expressions rest... (until '}')
             *
             *  Result expression creates and inits @p lhs_name,
             *  then evaluates expressions that follow definition
             *  up to same-level '}'
             **/
            static void start(const std::string & lhs_name,
                              const rp<Expression> & rhs,
                              parserstatemachine * p_psm);

            virtual void on_expr(ref::brw<Expression> expr,
                                 parserstatemachine * p_psm) override;

            virtual void on_rightbrace_token(const token_type & tk,
                                             parserstatemachine * p_psm) override;

        private:
            let1_xs(std::string lhs_name,
                    rp<Expression> rhs);

            /** named ctor idiom **/
            static std::unique_ptr<let1_xs> make(std::string lhs_name,
                                                 rp<Expression> rhs);

        private:
            /** name for new local variable **/
            std::string lhs_name_;
            /** set initial value for @ref lhs_name_ from value of this expression **/
            rp<Expression> rhs_;

            /** evaluate expressions in this sequence, in order, in environment
             *  with variable @ref lhs_name_ defined
             **/
            std::vector<rp<Expression>> expr_v_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/* end let1_xs.hpp */
