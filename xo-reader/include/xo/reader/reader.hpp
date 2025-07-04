/* file reader.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "parser.hpp"
#include "xo/expression/Expression.hpp"
#include "xo/tokenizer/tokenizer.hpp"

namespace xo {
    namespace scm {
        /** @class parse_result
         *  @brief Result object returned from reader::read_expr
         **/
        struct reader_result {
            using Expression = xo::ast::Expression;
            using span_type = span<const char>;

            reader_result(rp<Expression> expr, span_type rem)
                : expr_{std::move(expr)}, rem_{rem} {}

            /** parsed schematica expression **/
            rp<Expression> expr_;
            /** span giving text input consumed to construct expr,
             *  including any leading whitespace.
             *  This is the span returned in result of tokenizer<char>::scan()
             **/
            span_type rem_;
        };

        /**
         *  Use:
         *  @code
         *    reader rdr;
         *    rdr.begin_translation_unit()
         *
         *    bool eof = false;
         *    while (!eof) {
         *        auto input = ins.read_some();
         *        // eof: true if no more input will be forthcoming from this stream
         *        eof = ins.eof();
         *
         *        for (auto rem = input; !rem.empty();) {
         *            // res: (parsed-expr, used)
         *            auto res = rdr.read_expr(rem, eof);
         *
         *            if (res.first) {
         *                // do something with res.first (parsed expr)
         *                ...
         *            }
         *
         *            rem = rem.suffix_after(res.second);
         *        }
         *    }
         *
         *    // expect !rdr.has_prefix()
         *
         *  @endcode
         **/
        class reader {
        public:
            using tokenizer_type = tokenizer<char>;
            using span_type = tokenizer_type::span_type;

        public:
            reader() = default;

            /** call once before calling .read_expr()
             *  for a new interactive session
             **/
            void begin_interactive_session();

            /** counterpart to .begin_interactive_session()
             **/
            reader_result end_interactive_session();

            /** call once before calling .read_expr():
             *  1. with new reader
             *  2. if last read_expr() call had eof=true
             **/
            void begin_translation_unit();

            /** counterpart to .begin_translation_unit(),
             *  provided for symmetry's sake
             *
             *  Equivalent to:
             *  @code
             *    read_expr(span_type(nullptr, nullptr), true);
             *  @endcode
             **/
            reader_result end_translation_unit();

            /** Try to read one expression from @p input.
             *  Return struct containing parsed expression
             *  and span of characters comprising that expression
             *
             *  @param input  Supply this input span of chars
             *  @param eof.  True if input stream supplying @p input
             *         reports end-of-file immediately after the last char
             *         in @p input.
             **/
            reader_result read_expr(const span_type & input, bool eof);

        private:
            /** tokenizer: text -> tokens **/
            tokenizer_type tokenizer_;

            /** parser: tokens -> expressions **/
            parser parser_;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end reader.hpp */
