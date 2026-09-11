/** @file LogState.cpp **/

#include "LogState.hpp"
#include "PpSinkFactory.hpp"
#include <iostream>

namespace xo::pp {
    // ----- LogState -----

    PpSink &
    LogState::sink()
    {
        if (!sink_) {
            auto & f = PpSinkFactory::instance();

            this->builtin_flag_ = f.is_flat();
            this->sink_ = f.create();
        }

        return *sink_;
    }

    void
    LogState::set_sink(std::unique_ptr<PpSink> s)
    {
        this->builtin_flag_ = false;
        this->sink_ = std::move(s);
    }

    LogState &
    ThreadLogState::thread_log_state() {
        static thread_local LogState * s_state = nullptr;

        if (!s_state)
            s_state = new LogState();

        return *s_state;
    }

    void
    ThreadLogState::log_set_sink(std::unique_ptr<PpSink> s) {
        thread_log_state().set_sink(std::move(s));
    }
} /*namespace xo::pp*/

/* end LogState.cpp */
