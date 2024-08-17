/* file expect_type_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_type_xs.hpp"
#include "exprstate.hpp"
#include "xo/reflect/Reflect.hpp"

namespace xo {
    using xo::reflect::Reflect;

    namespace scm {

        std::unique_ptr<expect_type_xs>
        expect_type_xs::make() {
            return std::make_unique<expect_type_xs>(expect_type_xs());
        }

        expect_type_xs::expect_type_xs()
            : exprstate(exprstatetype::expect_type)
        {}

        void
        expect_type_xs::on_symbol_token(const token_type & tk,
                                        exprstatestack * p_stack,
                                        rp<Expression> * p_emit_expr)
        {
            const char * c_self_name = "expect_type_xs::on_symbol_token";

            TypeDescr td = nullptr;

            /* TODO: replace with typetable lookup */

            if (tk.text() == "f64")
                td = Reflect::require<double>();
            else if(tk.text() == "f32")
                td = Reflect::require<float>();
            else if(tk.text() == "i16")
                td = Reflect::require<std::int16_t>();
            else if(tk.text() == "i32")
                td = Reflect::require<std::int32_t>();
            else if(tk.text() == "i64")
                td = Reflect::require<std::int64_t>();

            if (!td) {
                throw std::runtime_error
                    (tostr(c_self_name,
                           ": unknown type name",
                           " (expecting f64|f32|i16|i32|i64)",
                           xtag("typename", tk.text())));
            }

            std::unique_ptr<exprstate> self = p_stack->pop_exprstate();
            p_stack->top_exprstate().on_typedescr(td, p_stack, p_emit_expr);
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end expect_type_xs.cpp */
