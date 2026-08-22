/** @file FlatSink.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpSink.hpp"
#include <ostream>

namespace xo::pp {
    /** @brief Degenerate PpSink that writes flat (unstructured) output to a std::ostream
     *
     *  Purpsoe:
     *  - adapt an uninstrumented ostream.
     *    Pretty-printing relies on a stream with an attached line accountant
     *    to track position relative to left margin.
     *  - adapter so that xo::pp::Prettifier<T> can serve both pretty-printing
     *    and legacy ostream printing.
     *
     *  Ignores group structure: {begin(), end(), split()} are no-ops.
     *  Output is equivalent to pretty printer behavior with an unbounded right margin.
     *
     *  Use:
     *  @code
     *    FlatSink sink(cout.rdbuf());
     *    Prettifier<Foo>::print(sink, foo);
     *  @endcode
     **/
    class FlatSink : public PpSink {
    public:
        using PpSink::split;
        using PpSink::newline;
        using uint32_t = std::uint32_t;
        using int32_t = std::int32_t;

    public:
        FlatSink(const PpStyle & style, std::streambuf * sbuf);
        explicit FlatSink(std::streambuf * sbuf)
            : PpSink(PpStyle::default_style()), sbuf_{sbuf}
        {}

        // inherited from PpSink

        /* keep the inherited split()/split(spaces) and newline()
         * convenience overloads visible alongside the overrides below
         */

        virtual PpSink & put(std::string_view x) override final;
        virtual PpSink & put_with_escape(std::string_view x, bool quote_flag) override final;
        virtual PpSink & begin() override final;
        virtual PpSink & begin(int32_t offset) override final;
        virtual PpSink & split(uint32_t spaces, int32_t offset) override final;
        virtual PpSink & newline(int32_t offset) override final;
        virtual PpSink & end() override final;
        virtual PpSinkInserter stream_open(uint32_t min_z) override final;
        virtual void stream_commit() override final;

    private:
        /** destination for flat output.  put()/split()/newline() go straight
         *  here via sputn()/sputc() -- no ostream, no sentry.
         **/
        std::streambuf * sbuf_ = nullptr;
    };
} /*namespace xo::pp*/

/* end FlatSink.hpp */
