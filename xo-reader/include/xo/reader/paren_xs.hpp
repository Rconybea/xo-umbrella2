/** @file paren_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"
//#include <cstdint>

namespace xo {
    namespace scm {
        enum class parenexprstatetype {
            invalid = -1,

            lparen_0,
            lparen_1,

            n_parenexprstatetype,
        };

        extern const char *
        parenexprstatetype_descr(parenexprstatetype x);

        inline std::ostream &
        operator<< (std::ostream & os, parenexprstatetype x) {
            os << parenexprstatetype_descr(x);
            return os;
        }

        /** @class paren_xs
         *  @brief state machine for handling parentheses in expressions
         **/
        class paren_xs : public exprstate {
        public:
            paren_xs();
            virtual ~paren_xs() = default;

            static const paren_xs * from(const exprstate * x) { return dynamic_cast<const paren_xs *>(x); }

            static void start(parserstatemachine * p_psm);

            bool admits_f64() const;
            bool admits_rightparen() const;

            virtual void on_expr(ref::brw<Expression> expr,
                                 parserstatemachine * p_psm) override;
            virtual void on_symbol(const std::string & symbol,
                                   parserstatemachine * p_psm) override;
            virtual void on_typedescr(TypeDescr td,
                                      parserstatemachine * p_psm) override;

            virtual void on_def_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;
            virtual void on_symbol_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;
            virtual void on_colon_token(const token_type & tk,
                                        parserstatemachine * p_psm) override;
            virtual void on_semicolon_token(const token_type & tk,
                                            parserstatemachine * p_psm) override;
            virtual void on_singleassign_token(const token_type & tk,
                                               parserstatemachine * p_psm) override;
            virtual void on_leftparen_token(const token_type & tk,
                                            parserstatemachine * p_psm) override;
            virtual void on_rightparen_token(const token_type & tk,
                                             parserstatemachine * p_psm) override;
            virtual void on_f64_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;

            virtual void print(std::ostream & os) const override;

        private:
            static std::unique_ptr<paren_xs> make();

        private:
            /**
             *  ( foo ... )
             *   ^
             *   |
             *   lparen_0
             **/
            parenexprstatetype parenxs_type_;
            /** populate expression (representing parenthesized value) here **/
            rp<Expression> gen_expr_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end paren_xs.hpp **/
