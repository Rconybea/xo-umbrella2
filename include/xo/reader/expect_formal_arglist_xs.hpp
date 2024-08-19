/* file expect_formal_arglist_xs.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "exprstate.hpp"
#include "formal_arg.hpp"
#include <vector>

namespace xo {
    namespace scm {
        /**
         *   ( name(1) : type(1) , ..., )
         *  ^ ^                 ^ ^    ^
         *  | |                 | |    |
         *  | |                 | |    argl_1b
         *  | argl_1a           | argla
         *  argl_0              argl_1b
         *
         *  argl_0 --on_leftparen_token()--> argl_1a
         *  argl_1a --on_formal()--> argl_1b
         *  argl_1b -+-on_comma_token()--> argl_1a
         *           \-on_rightparen_token()--> (done)
         **/
        enum class formalarglstatetype {
            invalid = -1,

            argl_0,
            argl_1a,
            argl_1b,

            n_formalarglstatetype,
        };

        extern const char *
        formalarglstatetype_descr(formalarglstatetype x);

        inline std::ostream &
        operator<< (std::ostream & os, formalarglstatetype x) {
            os << formalarglstatetype_descr(x);
            return os;
        }

        /** @class expect_formal_arglist
         *  @brief parser state-machine for a formal parameter list
         **/
        class expect_formal_arglist_xs : public exprstate {
        public:
            using Variable = xo::ast::Variable;

        public:
            expect_formal_arglist_xs();

            static void start(exprstatestack * p_stack);

            virtual void on_leftparen_token(const token_type & tk,
                                            exprstatestack * p_stack,
                                            rp<Expression> * p_emit_expr) override;
            virtual void on_formal(const rp<Variable> & formal,
                                   exprstatestack * p_stack,
                                   rp<Expression> * p_emit_expr) override;
            virtual void on_comma_token(const token_type & tk,
                                        exprstatestack * p_stack,
                                        rp<Expression> * p_emit_expr) override;
            virtual void on_rightparen_token(const token_type & tk,
                                             exprstatestack * p_stack,
                                             rp<Expression> * p_emit_expr) override;
            virtual void print(std::ostream & os) const override;

        private:
            static std::unique_ptr<expect_formal_arglist_xs> make();

        private:
            /** parsing state-machine state **/
            formalarglstatetype farglxs_type_ = formalarglstatetype::argl_0;
            /** populate with (parmaeter-name, parameter-type) list
             *  as they're encountered
             **/
            std::vector<rp<Variable>> argl_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/* end expect_formal_arglist_xs.hpp */
