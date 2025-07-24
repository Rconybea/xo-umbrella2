/* @file reader.cpp */

#include "reader.hpp"

namespace xo {
    namespace scm {
        reader::reader(bool debug_flag)
            : parser_{debug_flag}
        {}

        void
        reader::begin_interactive_session() {
            parser_.begin_interactive_session();
        }

        reader_result
        reader::end_interactive_session() {
            return this->read_expr(span_type(nullptr, nullptr), true /*eof*/);
        }

        void
        reader::begin_translation_unit() {
            parser_.begin_translation_unit();
        }

        reader_result
        reader::end_translation_unit() {
            return this->read_expr(span_type(nullptr, nullptr), true /*eof*/);
        }

        reader_result
        reader::read_expr(const span_type & input_arg, bool eof)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            span_type input = input_arg;

            /* input text-span consumed by this call.
             * Always comprises some number (possibly 0)
             * of complete tokens,  along with any leading
             * whitespace
             */
            span_type expr_span = input.prefix(0ul);

            while (!input.empty()) {
                /* each loop iterations reads one token */

                /* read one token from input */
                auto [tk, used_span, error1] = this->tokenizer_.scan2(input, eof);

                log && log(xtag("consumed", used_span));
                log && log(xtag("input.pre", input));

                input = this->tokenizer_.consume(used_span, input);
                expr_span += used_span;

                if (tk.is_valid()) {
                    /* forward just-read token to parser */
                    auto parser_result = this->parser_.include_token(tk);

                    if (parser_result.is_expression()) {
                        log && log(xtag("outcome", "victory!"),
                                   xtag("expr", parser_result.result_expr()));

                        /* token completes an expression -> victory */
                        return reader_result(parser_result.result_expr(),
                                             expr_span, parser_.stack_size(), reader_error());
                    } else if (parser_result.is_error()) {
                        /* 1. parser detected error.
                         * 2. tokenizer_.input_state() refers to position just after offending token
                         * 3. error_pos here is 0 because error detected at token boundary
                         */
                        reader_error error2(parser_result.error_src_function(),
                                            parser_result.error_description(),
                                            tokenizer_.input_state().rewind(tk.text().size()),
                                            0 /*error_pos*/);

                        std::cout << "parser error pre-report:" << std::endl;
                        error2.report(std::cout);

                        return reader_result(nullptr, expr_span, parser_.stack_size(), error2);
                    } else {
                        /* token did not complete an expression
                         * (e.g. token for '[')
                         *
                         * input span may contain more tokens -> iterate
                         */
                    }
                } else {
                    if (error1.is_error()) {
                        /* tokenizer detected an error */

                        std::cout << "tokenizer error pre-report:" << std::endl;
                        error1.report(std::cout);

                        return reader_result(nullptr, expr_span, parser_.stack_size(),
                                             reader_error(error1.src_function(),
                                                          error1.error_description(),
                                                          error1.input_state(),
                                                          error1.error_pos()));
                    } else {
                        /* control should not come here */

                        assert(input.empty());
                    }

                    /* need more tokens in input */
                    break;
                }
            }

            /* control here: eithero
             * 1. input.empty (perhaps ate some whitespace,  ok)
             * 2. missing or incomplete token (ok unless eof)
             */
            if (eof) {
                if (parser_.has_incomplete_expr()) {
                    throw std::runtime_error
                        ("reader::read_expr"
                         ": eof reached with incomplete expression");
                }

                if (tokenizer_.has_prefix()) {
                    throw std::runtime_error
                        ("reader::read_expr"
                         ": unintelligible input recognized at eof");
                }
            }

            log && log(xtag("outcome", "noop"),
                       xtag("parser.stack_size", parser_.stack_size()));

            return reader_result(nullptr, expr_span, parser_.stack_size(), reader_error());
        }

        void
        reader::reset_to_idle_toplevel()
        {
            this->tokenizer_.discard_current_line();
            this->parser_.reset_to_idle_toplevel();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end reader.cpp */
