/* @file define_xs.cpp */

#include "define_xs.hpp"

namespace xo {
    namespace scm {
        std::unique_ptr<define_xs>
        define_xs::def_0() {
            return std::make_unique<define_xs>(define_xs(DefineExprAccess::make_empty()));
        }

        define_xs::define_xs(rp<DefineExprAccess> def_expr)
            : exprstate(exprstatetype::defexpr),
              defxs_type_{defexprstatetype::def_0},
              def_expr_{std::move(def_expr)}
        {}

        bool
        define_xs::admits_definition() const
        {
            switch (defxs_type_) {

            case defexprstatetype::def_0:
            case defexprstatetype::def_1:
            case defexprstatetype::def_2:
            case defexprstatetype::def_3:
            case defexprstatetype::def_4:
            case defexprstatetype::def_5:
                /* note for def_4:
                 * rhs could certainly be a function body that contains
                 * nested defines; but then immediately-enclosing-exprstate
                 * would be a block
                 */
                return false;
            case defexprstatetype::invalid:
            case defexprstatetype::n_defexprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        bool
        define_xs::admits_symbol() const {
            switch (defxs_type_) {

            case defexprstatetype::def_0:
            case defexprstatetype::def_1:
            case defexprstatetype::def_2:
            case defexprstatetype::def_3:
            case defexprstatetype::def_4:
            case defexprstatetype::def_5:
                return false;

            case defexprstatetype::invalid:
            case defexprstatetype::n_defexprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        bool
        define_xs::admits_colon() const {
            switch (defxs_type_) {

            case defexprstatetype::def_0:
                return false;

            case defexprstatetype::def_1:
                return true;

            case defexprstatetype::def_2:
            case defexprstatetype::def_3:
            case defexprstatetype::def_4:
            case defexprstatetype::def_5:
                return false;

            case defexprstatetype::invalid:
            case defexprstatetype::n_defexprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        bool
        define_xs::admits_semicolon() const {
            switch (defxs_type_) {

            case defexprstatetype::def_0:
            case defexprstatetype::def_1:
            case defexprstatetype::def_2:
            case defexprstatetype::def_3:
            case defexprstatetype::def_4:
                return false;
            case defexprstatetype::def_5:
                return true;

            case defexprstatetype::invalid:
            case defexprstatetype::n_defexprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        bool
        define_xs::admits_singleassign() const {
            switch (defxs_type_) {

                /*
                 *   def foo       = 1 ;
                 *   def foo : f64 = 1 ;
                 *  ^   ^   ^ ^   ^ ^ ^
                 *  |   |   | |   | | (done)
                 *  |   |   | |   | def_4:expect_rhs_expression
                 *  |   |   | |   def_3
                 *  |   |   | def_2:expect_type
                 *  |   |   def_1
                 *  |   def_0:expect_symbol
                 *  expect_toplevel_expression_sequence
                 *
                 * note that we skip from def_1 -> def_4 if '=' instead of ':'
                 */
            case defexprstatetype::def_0:
                return false;

            case defexprstatetype::def_1:
                return true;

            case defexprstatetype::def_2:
                return false;

            case defexprstatetype::def_3:
                return true;

            case defexprstatetype::def_4:
            case defexprstatetype::def_5:
                return false;

            case defexprstatetype::invalid:
            case defexprstatetype::n_defexprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        bool
        define_xs::admits_leftparen() const {
            switch (defxs_type_) {

            case defexprstatetype::def_0:
            case defexprstatetype::def_1:
            case defexprstatetype::def_2:
            case defexprstatetype::def_3:
            case defexprstatetype::def_4:
            case defexprstatetype::def_5:
                /* input like
                 *   def foo : f64 = (
                 *      ^   ^ ^   ^ ^
                 *      |   | |   | def_4
                 *      |   | |   def_3
                 *      |   | def_2
                 *      |   def_1
                 *      def_0
                 *
                 * not allowed or relies on pushing another state
                 */
                return false;

            case defexprstatetype::invalid:
            case defexprstatetype::n_defexprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        bool
        define_xs::admits_rightparen() const {
            switch (defxs_type_) {

            case defexprstatetype::def_0:
            case defexprstatetype::def_1:
            case defexprstatetype::def_2:
            case defexprstatetype::def_3:
            case defexprstatetype::def_4:
            case defexprstatetype::def_5:
                return false;

            case defexprstatetype::invalid:
            case defexprstatetype::n_defexprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        void
        define_xs::on_expr(ref::brw<Expression> expr,
                           exprstatestack * /* p_stack */,
                           rp<Expression> * /* p_emit_expr */)
        {
            switch (this->defxs_type_) {

            case defexprstatetype::def_0:
            case defexprstatetype::def_1:
            case defexprstatetype::def_2:
            case defexprstatetype::def_3:
                /* NOT IMPLEMENTED */
                assert(false);
                return;
            case defexprstatetype::def_4: {
                /* have all the ingredients to create an expression
                 * representing a definition
                 *
                 * 1. if ir_type is a symbol,  interpret as variable name.
                 *    Need to be able to locate variable by type
                 * 2. if ir_type is an expression,  adopt as rhs
                 */
                rp<Expression> rhs_value = expr.promote();

                if (this->cvt_expr_)
                    this->cvt_expr_->assign_arg(rhs_value);
                else
                    this->def_expr_->assign_rhs(rhs_value);;

                rp<Expression> def_expr = this->def_expr_;

                this->defxs_type_ = defexprstatetype::def_5;
                return;
            }

            case defexprstatetype::def_5:
                assert(false);
                return;

            case defexprstatetype::invalid:
            case defexprstatetype::n_defexprstatetype:
                /* unreachable */
                assert(false);
                return;
            }
        }

        void
        define_xs::on_symbol(const std::string & symbol_name,
                             exprstatestack * /*p_stack*/,
                             rp<Expression> * /*p_emit_expr*/)
        {
            switch (this->defxs_type_) {
            case defexprstatetype::def_0:
                this->defxs_type_ = defexprstatetype::def_1;
                this->def_expr_->assign_lhs_name(symbol_name);
                //this->def_lhs_symbol_ = symbol_name;

                return;
            case defexprstatetype::def_1:
            case defexprstatetype::def_2:
            case defexprstatetype::def_3:
            case defexprstatetype::def_4:
            case defexprstatetype::def_5:
                /* NOT IMPLEMENTED */
                assert(false);
                return;

            case defexprstatetype::invalid:
            case defexprstatetype::n_defexprstatetype:
                /* unreachable */
                assert(false);
                return;
            }
        }

        void
        define_xs::on_typedescr(TypeDescr td,
                                exprstatestack * /*p_stack*/,
                                rp<Expression> * /*p_emit_expr*/)
        {
            switch (this->defxs_type_) {

            case defexprstatetype::def_0:
            case defexprstatetype::def_1:
                /* NOT IMPLEMENTED (ill-formed program) */
                assert(false);
                return;

            case defexprstatetype::def_2:
                this->defxs_type_ = defexprstatetype::def_3;
                this->cvt_expr_ = ConvertExprAccess::make(td /*dest_type*/,
                                                          nullptr /*source_expr*/);
                this->def_expr_->assign_rhs(this->cvt_expr_);
                //this->def_lhs_td_ = td;

                return;

            case defexprstatetype::def_3:
            case defexprstatetype::def_4:
            case defexprstatetype::def_5:
                /* NOT IMPLEMENTED */
                assert(false);
                return;

            case defexprstatetype::invalid:
            case defexprstatetype::n_defexprstatetype:
                /* unreachable */
                assert(false);
                return;
            }
        }

        void
        define_xs::on_colon_token(const token_type & /*tk*/,
                                  exprstatestack * p_stack)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_colon";

            /* lots of illegal states */
            if (!this->admits_colon())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected colon for parsing state",
                                               xtag("state", *this)));
            }

            if (this->defxs_type_ == defexprstatetype::def_1) {
                this->defxs_type_ = defexprstatetype::def_2;

                p_stack->push_exprstate(exprstate::expect_type());
            } else {
                assert(false);
            }
        }

        void
        define_xs::on_semicolon_token(const token_type & /*tk*/,
                                      exprstatestack * p_stack,
                                      rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_semicolon";

            if (!this->admits_semicolon())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected semicolon for parsing state",
                                               xtag("state", *this)));
            }

            if (this->defxs_type_ == defexprstatetype::def_5) {
                rp<Expression> expr = this->def_expr_;

                std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

                p_stack->top_exprstate().on_expr(expr,
                                                 p_stack,
                                                 p_emit_expr);
            } else {
                assert(false);
            }
        }

        void
        define_xs::on_singleassign_token(const token_type & /*tk*/,
                                         exprstatestack * p_stack)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_singleassign";

            if (!this->admits_singleassign())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected equals for parsing state",
                                               xtag("state", *this)));
            }

            if ((this->defxs_type_ == defexprstatetype::def_1)
                || (this->defxs_type_ == defexprstatetype::def_3))
            {
                this->defxs_type_ = defexprstatetype::def_4;

                p_stack->push_exprstate(exprstate::expect_rhs_expression());
            } else {
                assert(false);
            }
        }

        void
        define_xs::on_leftparen_token(const token_type & /*tk*/,
                                      exprstatestack * /*p_stack*/,
                                       rp<Expression> * /*p_emit_expr*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_leftparen";

            if (!this->admits_leftparen())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected leftparen '(' for parsing state",
                                               xtag("state", *this)));
            }

            assert(false); /* inserting this during refactor...? */
        }

        void
        define_xs::on_rightparen(exprstatestack * /*p_stack*/,
                                 rp<Expression> * /*p_emit_expr*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_rightparen";

            if (!this->admits_rightparen())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected rightparen ')' for parsing state",
                                               xtag("state", *this)));
            }

            assert(false); /* inserting this during refactor..? */
        }

        void
        define_xs::on_f64(const token_type & /*tk*/,
                          exprstatestack * /*p_stack*/,
                          rp<Expression> * /*p_emit_expr*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_f64";

            if (!this->admits_f64())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected floating-point literal for parsing state",
                                               xtag("state", *this)));
            }

            assert(false);
        }

        void
        define_xs::print(std::ostream & os) const {
            os << "<define_xs"
               << xtag("type", exs_type_);
            if (def_expr_)
                os << xtag("def_expr", def_expr_);
            if (cvt_expr_)
                os << xtag("cvt_expr", cvt_expr_);
            os << ">";
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end define_xs.cpp */
