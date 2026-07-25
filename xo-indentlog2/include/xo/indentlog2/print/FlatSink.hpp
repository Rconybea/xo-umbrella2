/** @file FlatSink.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpSink.hpp"
#include <ostream>

namespace xo::print {
    /** @brief Degenerate PpSink that writes flat (unstructured) output to a std::ostream
     *
     *  Purpsoe:
     *  - adapt an uninstrumented ostream.
     *    Pretty-printing relies on a stream with an attached line accountant
     *    to track position relative to left margin.
     *  - adapter so that xo::print::Pretty<T> can serve both pretty-printing
     *    and legacy ostream printing.
     *
     *  Ignores group structure: {begin(), end(), split()} are no-ops.
     *  Output is equivalent to pretty printer behavior with an unbounded right margin.
     *
     *  Use:
     *  @code
     *    FlatSink sink(cout);
     *    Pretty<Foo>::print(sink, foo);
     *  @endcode
     **/
    class FlatSink : public PpSink {
    public:
        explicit FlatSink(std::ostream & os) : os_{os} {}

        // inherited from PpSink

        virtual void put(std::string_view x) override final;
        virtual void begin() override final;
        virtual void split() override final;
        virtual void end() override final;
        virtual PpSinkInserter stream_open(uint32_t min_z) override final;
        virtual void stream_commit() override final;

    private:
        /** destination for flat output **/
        std::ostream & os_;
    };
} /*namespace xo::print*/

/* end FlatSink.hpp */
