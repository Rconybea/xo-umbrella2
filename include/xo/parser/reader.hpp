/* file reader.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "parser.hpp"
#include "xo/tokenizer/tokenizer.hpp"

namespace xo {
    namespace scm {
        /**
         *  Use:
         *  @code
         *    reader rdr;
         *
         *    bool eof = false;
         *    while (!eof) {
         *        auto input = ins.read_some();
         *        // eof: true if no more input will be forthcoming from this stream
         *        eof = ins.eof();
         *
         *        for (auto rem = input; ; !rem.empty()) {
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
            reader() = default;

        private:
            /** tokenizer: text -> tokens **/
            tokenizer tokenizer_;

            /** parser: tokens -> expressions (TODO: reanme ->reader) **/
            parser parser_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/* end Repl.hpp */
