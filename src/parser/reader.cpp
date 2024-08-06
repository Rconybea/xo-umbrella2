/* @file reader.cpp */

#include "reader.hpp"

namespace xo {
    namespace scm {
        reader_result
        reader::read_expr(const span_type & input_arg, bool eof)
        {
            span_type input = input_arg;

            /* input text-span consumed by this call.
             * Always comprises some number (possibly 0)
             * of complete tokens,  along with any leading
             * whitespace
             */
            span_type expr_span = input.prefix(0ul);

            while (!input.empty()) {
                /* read one token from input */
                auto sr = this->tokenizer_.scan2(input, eof);
                const auto & tk = sr.first;
                const span_type & used_span = sr.second;

                input = input.after_prefix(used_span);
                expr_span += used_span;

                if (tk.is_valid()) {
                    /* forward just-read token to parser */
                    auto expr = this->parser_.include_token(tk);

                    if (expr) {
                        /* token completes an expression -> victory */
                        return reader_result(expr, expr_span);
                    } else {
                        /* token did not complete an expression
                         * (e.g. token for '[')
                         *
                         * input span may contain more tokens -> iterate
                         */
                        input = input.after_prefix(used_span);
                    }
                } else {
                    assert(input.empty());

                    /* no more tokens in input */
                    break;
                }
            }

            /* control here: either
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

            return reader_result(nullptr, expr_span);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end reader.cpp */
