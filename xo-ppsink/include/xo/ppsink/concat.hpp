/** @file concat.hpp
 *
 *  concat(args...): a renderable that emits its arguments back-to-back, with
 *  no separator and no intermediate string.
 *
 *      st.field(concat("[", i, "]"), stack_[i]);
 *
 *  The point is what it does NOT do.  The idiom it replaces,
 *
 *      st.field(tostr("[", i, "]"), stack_[i]);
 *
 *  constructs a std::stringstream and a std::string per call (see tostr.hpp)
 *  purely to hand the bytes straight back to the sink that could have written
 *  them.  concat() holds the arguments and renders them where the result is
 *  wanted, so nothing is buffered.
 *
 *  DIFFERENCES FROM legacy xo::concat (xo/indentlog/print/concat.hpp):
 *
 *  1. variadic, not binary.
 *  2. captures BY REFERENCE, not by value -- that is the whole point; see the
 *     lifetime rule below.
 *  3. NOT atomic.  Legacy declared concat_impl a pretty-printing leaf
 *     ("don't want structure visible to the pretty-printer").  Here each
 *     argument goes through sink.pp(), so a structured argument emits its
 *     structure, including group breaks.  In practice concat is used for short
 *     flat text -- labels, prefixes -- where every argument is a scalar or a
 *     string and no splits are produced.  Passing a struct-shaped argument
 *     works but is probably a mistake: prefer pretty_struct/struct_open.
 *
 *  LIFETIME: like field() (pretty_struct.hpp), concat() stores POINTERS to its
 *  arguments.  A temporary argument lives to the end of the full-expression, so
 *
 *      st.field(concat("[", i, "]"), v);      // fine: rendered within the call
 *
 *  is safe, while hoisting the concat out of the call is not:
 *
 *      auto label = concat("[", i, "]");      // DANGLING once the temporaries die
 *
 *  Prefer building it in the expression that consumes it.
 **/

#pragma once

#include "pretty.hpp"
#include <tuple>

namespace xo::pp {
    /** @brief holds references to a pack of renderables; see concat() **/
    template <typename... Ts>
    class concat_impl {
    public:
        explicit concat_impl(const Ts &... args) : args_{&args...} {}

        void render(PpSink & sink) const {
            std::apply([&sink](const Ts *... p) { (sink.pp(*p), ...); }, args_);
        }

    private:
        /** pointers, not copies: see the lifetime rule in the file comment **/
        std::tuple<const Ts *...> args_;
    }; /*concat_impl*/

    /** render @p args back-to-back with no separator and no intermediate
     *  string.  Arguments are captured BY REFERENCE -- build it in the
     *  expression that consumes it.
     **/
    template <typename... Ts>
    concat_impl<Ts...>
    concat(const Ts &... args) {
        return concat_impl<Ts...>(args...);
    }

    template <typename... Ts>
    struct Prettifier<concat_impl<Ts...>> {
        static void print(PpSink & sink, const concat_impl<Ts...> & x) {
            x.render(sink);
        }
    };
} /*namespace xo::pp*/

/* end concat.hpp */
