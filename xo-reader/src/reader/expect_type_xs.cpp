/* file expect_type_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_type_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
#include "xo/reflect/Reflect.hpp"

namespace xo {
    using xo::reflect::Reflect;

    namespace scm {

        std::unique_ptr<expect_type_xs>
        expect_type_xs::make() {
            return std::make_unique<expect_type_xs>(expect_type_xs());
        }

        void
        expect_type_xs::start(parserstatemachine * p_psm) {
            p_psm->push_exprstate(expect_type_xs::make());
        }

        expect_type_xs::expect_type_xs()
            : exprstate(exprstatetype::expect_type)
        {}

        const char *
        expect_type_xs::get_expect_str() const
        {
            return "typename";
        }

        void
        expect_type_xs::on_symbol_token(const token_type & tk,
                                        parserstatemachine * p_psm)
        {
            const char * c_self_name = "expect_type_xs::on_symbol_token";

            TypeDescr td = nullptr;

            /* TODO: replace with typetable lookup */

            if (tk.text() == "bool")
                td = Reflect::require<bool>();
            else if (tk.text() == "str")
                td = Reflect::require<std::string>();
            else if (tk.text() == "f64")
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
                const char * exp = get_expect_str();

                std::string errmsg = tostr("expect_type_xs: unexpected token for parsing state",
                                           xtag("expecting", exp),
                                           xtag("token", tk.tk_type()),
                                           xtag("text", tk.text()),
                                           xtag("state", this->exs_type()));

                p_psm->on_error(c_self_name, std::move(errmsg));
            }

            std::unique_ptr<exprstate> self = p_psm->pop_exprstate();
            p_psm->top_exprstate().on_typedescr(td, p_psm);
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end expect_type_xs.cpp */
