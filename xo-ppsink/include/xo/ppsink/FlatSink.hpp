/** @file FlatSink.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpSink.hpp"
#include <streambuf>
#include <sstream>

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
        /** @p sbuf null selects in-memory output: the sink accumulates into
         *  a buffer of its own, which @ref copy_output reports and @ref
         *  complete reclaims.  That is what PpSinkFactory::SinkOutput::
         *  k_memory asks for.
         **/
        FlatSink(const PpStyle & style, std::streambuf * sbuf);
        explicit FlatSink(std::streambuf * sbuf)
            : PpSink(PpStyle::default_style()),
              sbuf_{sbuf ? sbuf : &own_sbuf_}
        {}
        ~FlatSink() = default;

        // inherited from PpSink

        /* keep the inherited split()/split(spaces) and newline()
         * convenience overloads visible alongside the overrides below
         */

        virtual std::pair<bool, std::string> copy_output() override final;
        virtual PpSink & put(std::string_view x) override final;
        virtual PpSink & put_with_escape(std::string_view x, bool quote_flag) override final;
        virtual PpSink & begin() override final;
        virtual PpSink & begin(int32_t offset) override final;
        virtual PpSink & split(uint32_t spaces, int32_t offset) override final;
        virtual PpSink & newline(int32_t offset) override final;
        virtual PpSink & end() override final;
        virtual PpSink & complete() override final;
        virtual PpSinkInserter stream_open(uint32_t min_z) override final;
        virtual void stream_commit() override final;

    private:
        /** true when output goes to @ref own_sbuf_ rather than a caller's
         *  streambuf -- i.e. this sink was made for in-memory output
         **/
        bool is_memory_sink() const { return sbuf_ == &own_sbuf_; }

    private:
        /** in-memory destination, used when the caller supplied none.
         *
         *  Declared BEFORE sbuf_: sbuf_ may be initialised to point at it.
         **/
        std::stringbuf own_sbuf_;
        /** destination for flat output.  put()/split()/newline() go straight
         *  here via sputn()/sputc() -- no ostream, no sentry.
         *
         *  Never null: a null constructor argument selects @ref own_sbuf_.
         **/
        std::streambuf * sbuf_ = nullptr;
    };
} /*namespace xo::pp*/

/* end FlatSink.hpp */
