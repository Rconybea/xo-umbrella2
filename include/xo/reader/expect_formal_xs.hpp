/* file expect_formal_xs.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "exprstate.hpp"
#include "formal_arg.hpp"

namespace xo {
    namespace scm {
        /**
         *   name : type
         *  ^    ^ ^
         *  |    | formal_2
         *  |    formal_1
         *  formal_0
         *
         *  formal_0 --on_symbol()--> formal_1
         *  formal_1 --on_colon_token()--> formal_2
         *  formal_2 --on_typedescr()--> (done)
         **/
        enum class formalstatetype {
            invalid = -1,

            formal_0,
            formal_1,
            formal_2,

            n_formalstatetype,
        };

        extern const char *
        formalstatetype_descr(formalstatetype x);

        inline std::ostream &
        operator<< (std::ostream & os, formalstatetype x) {
            os << formalstatetype_descr(x);
            return os;
        }

        /** @class expect_formal_xs
         *  @brief parser state-machine for a typed formal parameter
         **/
        class expect_formal_xs : public exprstate {
        public:
            expect_formal_xs();

            static void start(exprstatestack * p_stack);

            virtual void on_symbol(const std::string & symbol_name,
                                   exprstatestack * p_stack,
                                   rp<Expression> * p_emit_expr) override;

            virtual void on_colon_token(const token_type & tk,
                                        parserstatemachine * p_psm) override;

            // virtual void on_comma_token(...) override;

#ifdef PROBABLY_NOT
            virtual void on_rightparen_token(const token_type & tk,
                                             exprstatestack * p_stack,
                                             rp<Expression> * p_emit_expr) override;
#endif

            virtual void on_typedescr(TypeDescr td,
                                      parserstatemachine * p_psm) override;

            virtual void print(std::ostream & os) const override;

        private:
            static std::unique_ptr<expect_formal_xs> make();

        private:
            /** parsing state-machine state **/
            formalstatetype formalxs_type_ = formalstatetype::formal_0;
            /** populate with {parameter-name, parameter-type}
             *  as they're encountered
             **/
            formal_arg result_;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end expect_formal_xs.hpp */
