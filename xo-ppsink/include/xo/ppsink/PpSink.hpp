/** @file PpSink.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpStyle.hpp"
#include <string_view>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <optional>

namespace xo::pp {
    class PpSink;

    /** @brief specify default sink for logging.
     *
     *  In practice will be either FlatSink (simple, low-level)
     *  or PrettySink (pretty-printing).
     *
     *  Design Note: using singleton pattern here because this feature
     *  arrives at very low-level, before we have app context
     **/
    class SinkFactory {
    public:
        static SinkFactory & instance() { return *s_instance; }
        /** establish factory instance **/
        static SinkFactory & set_instance(SinkFactory * x);

        /** FlatSinkFactory -> true; PrettySinkFactory -> false **/
        virtual bool is_flat() const = 0;

        /** create sink **/
        virtual std::unique_ptr<PpSink> create() = 0;

    private:
        /** (singleton) default sink factory **/
        static SinkFactory * s_instance;
    };

    /** @brief RAII convenience class to ensure ppsink.stream_commit() invoked
     **/
    class PpSinkInserter {
    public:
        explicit PpSinkInserter(PpSink * pps, std::streambuf * sbuf);
        PpSinkInserter(const PpSinkInserter &) = delete;
        ~PpSinkInserter();

        std::streambuf * sbuf() { return sbuf_; }

        /** finalizer.  Will be called at least once. Idempotent; dtor invokes **/
        void finish();

        PpSinkInserter & operator= (const PpSinkInserter & x) = delete;

    private:
        /** pretty-printer api **/
        PpSink * ppsink_ = nullptr;
        /** stream api.  Writing to @ref stream_ appends to @ref sink_ **/
        std::streambuf * sbuf_ = nullptr;
    };

    /** RAII scope for a struct with a runtime number of fields; defined in
     *  pretty_struct.hpp.  Named here so PpSink::struct_open() can return it.
     **/
    class struct_scope;

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
        explicit PpSink(const PpStyle & style);
        virtual ~PpSink();

        /** pretty-print @p x to this sink
         *  see pretty.hpp for implementation
         **/
        template <typename T>
        PpSink & pp(const T & x);

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

        /** apply dwim() to each argument, left-to-right:
         *    sink("f(", begin(2), x, split(0,0), y, end, ")");
         *  structuring descriptors (begin/split/newline/end) drive layout;
         *  everything else prints via pp().  See pretty.hpp for implementation.
         **/
        template <typename... Ts>
        void operator()(const Ts &... args);

        /** render a struct-shaped value as
         *    <Name :f1 v1 :f2 v2>
         *  breaking, when it does not fit, to
         *    <Name
         *      :f1 v1
         *      :f2 v2>
         *
         *  Name each field with field() (see pretty_struct.hpp); a field whose
         *  present() is false is omitted entirely.  The ppsink replacement for
         *  legacy ppstate::pretty_struct + refrtag.
         *
         *  See pretty_struct.hpp for implementation.
         **/
        template <typename... Fields>
        void pretty_struct(std::string_view name, const Fields &... fields);

        /** open a struct whose fields are added one at a time, for a field
         *  count that is only known at runtime:
         *
         *    {
         *        auto st = sink.struct_open("stack", stack.size() > 1);
         *        st.field("size", stack.size());
         *        for (std::size_t i = 0; i < stack.size(); ++i)
         *            st.field(concat("[", i, "]"), stack[i]);
         *    }   // ">" and end() emitted here
         *
         *  pretty_struct() is this with a compile-time field list; reach for
         *  struct_open() only when a loop contributes fields.
         *
         *  @p force_break: separate fields with a forced break rather than an
         *  optional one, so the struct always renders multi-line even where it
         *  would fit.  For a deliberate layout policy -- NOT a substitute for
         *  the sink's own fit decision.
         *
         *  See pretty_struct.hpp for implementation.
         **/
        struct_scope struct_open(std::string_view name, bool force_break = false);

        /** write string comprising contents of @p s **/
        virtual PpSink & put(std::string_view s) = 0;

        /** write @p s as a single token, with escapes expanded per escape.hpp
         *  (backslash, dquote, newline, cr, and control chars as \xNN),
         *  surrounded by Escape::c_quote iff @p quote_flag.
         *
         *  Deciding *whether* to quote (never / only when ambiguous / always)
         *  is a policy layered on top; applying the quotes has to happen here,
         *  since they belong in the same token as the content.
         *
         *  Not defaulted in terms of put(): the whole expansion must land in
         *  exactly one token, or the pretty-printer could break a line in the
         *  middle of an escaped string -- or between a quote and what it quotes.
         *
         *  @p quote_flag is deliberately not a defaulted argument: defaults on
         *  virtuals are taken from the static type, so overrides can silently
         *  disagree.
         **/
        virtual PpSink & put_with_escape(std::string_view s, bool quote_flag) = 0;

        /** begin group of nested items; indent one nesting level.
         *  Sequence begin,[string*,split]*,end
         **/
        virtual PpSink & begin() = 0;
        /** begin a group; @p offset adds to the running indent until end() **/
        virtual PpSink & begin(std::int32_t offset) = 0;
        /** begin a group, use current output column for indent origin
         *  (instead of running indent). @p offset then adds to that origin.
         *
         *  Requires a sink with output-column accounting (i.e. PrettySink).
         *  Otherwise fallback to plain begin() (e.g. FlatSink).
         **/
        virtual PpSink & begin_here(std::int32_t offset) { return this->begin(offset); }

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

        /** finish the current record (log line/banner): the caller signals a
         *  record boundary here instead of emitting a terminating newline
         *  itself.  Default: emit a newline via put(), preserving the plain
         *  streaming behavior.  Record-buffering sinks (e.g. PrettySink)
         *  override to also drain + reclaim at this boundary.
         **/
        virtual PpSink & complete() { return this->put("\n"); }

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

        /** @defgroup ppsink-style presentation style **/
        ///@{

        /** the presentation choices in force for this sink -- see PpStyle.hpp.
         *
         *  Held here, at PpSink level, rather than in PpConfig: the Prettifiers
         *  that consume it (tag.hpp, pretty_struct.hpp) are handed only a
         *  PpSink &, PpConfig lives in a subsystem ABOVE this one, and a
         *  FlatSink has no PpConfig at all.
         *
         *  Not virtual on purpose: every sink gets this behaviour without
         *  writing anything, and there is one place the value can come from.
         **/
        const PpStyle & style() const { return style_; }

        ///@}

    protected:
        /** initialised from the process-wide defaults AT CONSTRUCTION -- so
         *  changing PpStyle::default_style() afterwards does not reach a sink
         *  that already exists.
         **/
        PpStyle style_;
    };


} /*namespace xo::pp*/

/* end PpSink.hpp */
