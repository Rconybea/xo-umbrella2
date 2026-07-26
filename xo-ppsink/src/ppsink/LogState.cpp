/** @file LogState.cpp **/

#include <xo/ppsink/LogState.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <iostream>

namespace xo::print {
    namespace {
        /** process-wide default sink: flat output to std::clog.
         *  (POC: whole-program FlatSink; per-thread interleaving not yet addressed)
         **/
        FlatSink &
        default_sink() {
            static FlatSink s_default_sink(std::clog);
            return s_default_sink;
        }
    } /*namespace*/

    PpSink &
    LogState::sink() {
        if (!sink_)
            sink_ = &default_sink();
        return *sink_;
    }

    LogState &
    ThreadLogState::thread_log_state() {
        static thread_local LogState s_state;
        return s_state;
    }

    void
    ThreadLogState::log_set_sink(PpSink * s) {
        thread_log_state().set_sink(s);
    }
} /*namespace xo::print*/

/* end LogState.cpp */
