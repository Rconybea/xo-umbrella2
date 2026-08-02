/** @file LogState.cpp **/

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/LogState.hpp>
#include <iostream>

namespace xo::pp {
    /** process-wide default sink: flat output to std::clog.
     *  (POC: whole-program FlatSink; per-thread interleaving not yet addressed)
     **/
    FlatSink *
    require_default_sink() {
        /** low-dependency fallback. No pretty-printing **/
        static FlatSink s_default_sink(std::clog);

        return &s_default_sink;
    } /*namespace*/

    PpSink &
    LogState::sink()
    {
        if (!sink_) {
            this->builtin_flag_ = true;
            this->sink_ = require_default_sink();
        }

        return *sink_;
    }

    void
    LogState::set_sink(PpSink * s) {
        builtin_flag_ = false;
        sink_ = s;
    }

    LogState &
    ThreadLogState::thread_log_state() {
        static thread_local LogState * s_state = nullptr;

        if (!s_state)
            s_state = new LogState();

        return *s_state;
    }

    void
    ThreadLogState::log_set_sink(PpSink * s) {
        thread_log_state().set_sink(s);
    }
} /*namespace xo::pp*/

/* end LogState.cpp */
