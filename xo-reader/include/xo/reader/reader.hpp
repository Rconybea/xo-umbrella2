/* file reader.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "parser.hpp"
#include "reader_error.hpp"
#include "xo/expression/Expression.hpp"
#include "xo/expression/GlobalSymtab.hpp"
#include "xo/expression/pretty_expression.hpp"
#include "xo/tokenizer/tokenizer.hpp"

namespace xo {
    namespace scm {
        /** @class parse_result
         *  @brief Result object returned from reader::read_expr
         **/
        struct reader_result {
            using Expression = xo::scm::Expression;
            using span_type = span<const char>;

            reader_result(rp<Expression> expr, span_type rem, std::size_t psz, const reader_error & error)
                : expr_{std::move(expr)}, rem_{rem}, parser_stack_size_{psz}, error_{error} {}

            /** true if reader parsed a complete expression **/
            bool expr_complete() const { return expr_.get(); }

            /** parsed schematica expression **/
            rp<Expression> expr_;
            /** span giving text input consumed to construct expr,
             *  including any leading whitespace.
             *  This is the span returned in result of tokenizer<char>::scan()
             **/
            span_type rem_;

            /** parser nesting level when this result delivered
             *  will be zero whenever @ref expr_ is non-null
             **/
            std::size_t parser_stack_size_ = 0;

            /** error description, whenever .error_.is_error() is true **/
            reader_error error_;
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
         *            auto [expres = rdr.read_expr(rem, eof);
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
            reader(const rp<GlobalSymtab> & toplevel_symtab, bool debug_flag);

            bool debug_flag() const { return parser_.debug_flag(); }

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

            /** reset to known starting point after encountering an error.
             *  - remainder of stashed current line.
             *    Necesary for well-formatted error reporting.
             *  - current parsing state
             **/
            void reset_to_idle_toplevel();

        private:
            /** tokenizer: text -> tokens **/
            tokenizer_type tokenizer_;

            /** parser: tokens -> expressions **/
            parser parser_;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end reader.hpp */
