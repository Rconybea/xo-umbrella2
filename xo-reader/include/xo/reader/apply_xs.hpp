/* @file apply_xs.hpp */

#pragma once

#include "exprstate.hpp"
#include "xo/expression/Apply.hpp"


namespace xo {
    namespace scm {
        /**
         *    fn ( arg1 , arg2 , .. , argn )
         *   ^  ^ ^    ^ ^    ^      ^    ^ ^
         *   |  | |    | |    |      |    | (done)
         *   |  | |    | |    |      |    apply_3
         *   |  | |    | |    |      apply_2:expect_rhs_expression
         *   |  | |    | |    apply_3
         *   |  | |    | apply_2:expect_rhs_expression
         *   |  | |    apply_3
         *   |  | apply_2:expect_rhs_expression
         *   |  apply_1
         *   apply_0:expect_rhs_expression
         *
         *   apply_0 --on_expr()--> apply_1
         *   apply_1 --on_leftparen()--> apply_2
         *   apply_2 --on_expr()--> apply_3
         *   apply_3 --on_comma()--> apply_2
         *           --on_rightparen()-> (done)
         *
         *   apply_0: start
         *   apply_1: leftparen following expr allows parser to recognize apply
         *   apply_2: expect next argument
         *   apply_3: got argument, expect comma or rightparen to continue
         *   (done): apply complete, pop exprstate from stack
         *
         * In practice will start in state apply_1
         **/
        enum class applyexprstatetype {
            invalid = -1,

            apply_0,
            apply_1,
            apply_2,
            apply_3,

            n_applyexprstatetype
        };

        extern const char * applyexprstatetype_descr(applyexprstatetype x);

        std::ostream &
        operator<<(std::ostream & os, applyexprstatetype x);

        /** @class apply_xs
         *  @brief state machine for parsing a schematic function-call-expression
         *
         */
        class apply_xs : public exprstate {
        public:
            using Apply = xo::scm::Apply;

        public:
            explicit apply_xs();
            virtual ~apply_xs() = default;

            /** downcast from parent type **/
            static const apply_xs * from(const exprstate * x) {
                return dynamic_cast<const apply_xs *>(x);
            }

            /**
             * Start apply. Will trigger this after input like
             *   "fn("
             *
             * apply_xs remains on expr stack until closing right paren
             *    fn(arg1-expr, arg2-expr, ...)
             *
             * @p fnex   expression in function position
             * @p p_psm  parser state machine
             **/
            static void start(rp<Expression> fnex,
                              parserstatemachine * p_psm);

            virtual const char * get_expect_str() const override;

            virtual void on_expr(bp<Expression> expr,
                                 parserstatemachine * p_psm) override;

            virtual void on_comma_token(const token_type & tk,
                                        parserstatemachine * p_psm) override;
            virtual void on_leftparen_token(const token_type & tk,
                                             parserstatemachine * p_psm) override;
            virtual void on_rightparen_token(const token_type & tk,
                                             parserstatemachine * p_psm) override;

            virtual void print(std::ostream & os) const override;
            virtual bool pretty_print(const print::ppindentinfo & ppii) const final override;

        private:
            static std::unique_ptr<apply_xs> make();

        private:
            /** current state of parser for this apply expression **/
            applyexprstatetype applyxs_type_ = applyexprstatetype::apply_0;
            /** evaluates to function to be invoked **/
            rp<Expression> fn_expr_;
            /** evaluates to the arguments to pass to @ref fn_ **/
            std::vector<rp<Expression>> args_expr_v_;
        };
    } /*namespace scm */

    namespace print {
#ifndef ppdetail_atomic
        PPDETAIL_ATOMIC(xo::scm::applyexprstatetype);
#endif
    }
} /*namespace xo*/

/* end apply_xs.hpp */
