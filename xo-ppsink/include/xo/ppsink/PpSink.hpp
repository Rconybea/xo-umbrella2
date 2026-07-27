/** @file PpSink.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include <string_view>
// Deliberately not including <ostream> here.
// This file pulled into satellites that need
// to invoke PpSink methods.
// We don't want to force code that supports
// pretty-printing to #include <ostream>.
#include <iosfwd>
#include <optional>
#include <cstddef>
#include <cstdint>

namespace xo::pp {
    class PpSink;

    /** @brief RAII convenience class to ensure ppsink.stream_commit() invoked
     **/
    class PpSinkInserter {
    public:
        explicit PpSinkInserter(PpSink * pps, std::ostream * os);
        PpSinkInserter(const PpSinkInserter &) = delete;
        ~PpSinkInserter();

        std::ostream & os() { return *os_; }

        /** finalizer.  Will be called at least once. Idempotent; dtor invokes **/
        void finish();

        PpSinkInserter & operator= (const PpSinkInserter & x) = delete;

    private:
        /** pretty-printer api **/
        PpSink * ppsink_ = nullptr;
        /** stream api.  Writing to @ref stream_ appends to @ref sink_ **/
        std::ostream * os_ = nullptr;
    };

    /** convenience so given a PpSinkInserter:
     *    PpSinkInserter ins = ppsink.stream_open(..);
     *  we can write
     *    ins << x << ...;
     *  instead of
     *    ins.os() << x << ...;
     **/
    template <typename T>
    inline PpSinkInserter &
    operator<<(PpSinkInserter & ins, const T & x) {
        ins.os() << x;
        return ins;
    }

    /** @brief Interface for a stream with pretty-printing support
     *
     *  Defer using faceted object model, so we can use this
     *  pretty-printing during implementation of xo-facet itself.
     *  May revisit later.
     **/
    class PpSink {
    public:
        using uint32_t = std::uint32_t;

    public:
        /** pretty-print @p x to this sink
         *  see pretty.hpp for implementation
         **/
        template <typename T>
        void pp(const T & x);

        /** do-what-i-mean structuring convenience
         *  for custom-printer authors.
         *  See pretty.hpp for implementation.
         *
         *  PpSink & sink = ...;
         *
         *  sink.dwim(begin(0));
         *  sink.dwim(split(0, 1));
         *  sink.dwim(newline(2));
         *  sink.dwim(end);
         *
         **/
        template <typename Dwim>
        void dwim(const Dwim & x);

        /** write string comprising contents of @p s **/
        virtual PpSink & put(std::string_view s) = 0;

        /** begin group of nested items; indent one nesting level.
         *  Sequence begin,[string*,split]*,end
         **/
        virtual PpSink & begin() = 0;
        /** begin a group; @p offset adds to the running indent until end() **/
        virtual PpSink & begin(std::int32_t offset) = 0;

        /** optional line break.
         *  Group fits: emit @p spaces spaces.
         *  Group breaks: newline + (running_indent + @p offset).
         **/
        virtual PpSink & split(std::uint32_t spaces, std::int32_t offset) = 0;
        /** optional break with no extra offset **/
        PpSink & split(std::uint32_t spaces) { return this->split(spaces, 0); }
        /** optional zero-width break **/
        PpSink & split() { return this->split(0, 0); }

        /** forced break: always newline + (running_indent + @p offset),
         *  forcing every enclosing group to break
         **/
        virtual PpSink & newline(std::int32_t offset) = 0;
        /** forced break with no extra offset **/
        PpSink & newline() { return this->newline(0); }

        /** end group of nested items previously  introduced with begin() **/
        virtual PpSink & end() = 0;

        /** current visible output column, if this sink tracks one.
         *  PrettySink does (via its line accountant); FlatSink does not and
         *  returns nullopt -- the caller then falls back to inline placement
         *  for right-aligned fields (e.g. a code location).
         **/
        virtual std::optional<std::size_t> lpos() const { return std::nullopt; }
        /** temporary stream to fill a string token.
         *  Token will be completed by next call to stream_commit().
         *  Example:
         *    auto ins = sink.stream_open(256);
         *    ins.os() << foo << ..;
         *    ins.finish();
         *  is equivalent (in effect) to
         *    stringstream ss;
         *    ss << foo << ..;
         *    sink.put(ss.str());
         *  but avoids heap allocation + copy
         **/
        virtual PpSinkInserter stream_open(uint32_t min_z) = 0;
        /** complete string started by stream_open() **/
        virtual void stream_commit() = 0;

    };


} /*namespace xo::pp*/

/* end PpSink.hpp */
