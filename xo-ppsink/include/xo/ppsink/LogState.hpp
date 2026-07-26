/** @file LogState.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Arena-free per-thread logging state: nesting level + the active PpSink.
 *  This is the frontend state that (in the new levelization) replaces the
 *  WIP LogState/LogBuffer machinery in xo-indentlog2 -- the arena-backed
 *  render buffer stays inside PrettySink; only nesting + "which sink" live here.
 **/

#pragma once

#include "PpSink.hpp"
#include <cstdint>

namespace xo::print {
    /** @brief per-thread logging state: nesting level + the active sink.
     *
     *  Per-thread so one thread's line doesn't interleave with another's;
     *  the sink *type* is a whole-program choice (POC: FlatSink; later a
     *  per-thread PrettySink for arena-backed pretty output).
     **/
    class LogState {
    public:
        std::uint32_t nesting_level() const { return nesting_; }
        void incr_nesting() { ++nesting_; }
        void decr_nesting() { if (nesting_ > 0) --nesting_; }

        /** active sink for this thread (default: a FlatSink over std::clog) **/
        PpSink & sink();
        /** override the active sink (e.g. a capture sink in tests, or a
         *  PrettySink once xo-indentlog2 is available).  nullptr restores default.
         **/
        void set_sink(PpSink * s) { sink_ = s; }

    private:
        /** current scope nesting depth for this thread; drives indentation **/
        std::uint32_t nesting_ = 0;
        /** sink log output is written to; nullptr => the process default flat sink **/
        PpSink * sink_ = nullptr;
    };

    /** @brief static accessors for the calling thread's @ref LogState **/
    class ThreadLogState {
    public:
        /** the calling thread's logging state **/
        static LogState & thread_log_state();

        /** set (or clear, with nullptr) the active sink for the calling thread **/
        static void log_set_sink(PpSink * s);
    };
} /*namespace xo::print*/

/* end LogState.hpp */
