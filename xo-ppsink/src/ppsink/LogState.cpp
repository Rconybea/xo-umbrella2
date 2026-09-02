/** @file LogState.cpp **/

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/LogState.hpp>
#include <iostream>

namespace xo::pp {
    /** process-wide default sink: flat output to std::clog.
     *  (POC: whole-program FlatSink; per-thread interleaving not yet addressed)
     **/
    class FlatSinkFactory : public SinkFactory {
    public:
        /** fallback factory: creates flat sinks **/
        virtual bool is_flat() const override { return true; }
        /** create FlatSink instance **/
        virtual std::unique_ptr<PpSink> create() override {
            /** low-dependency fallback. No pretty-printing **/
            return std::make_unique<FlatSink>(std::clog.rdbuf());
        }
    };

    FlatSinkFactory s_flatsink_factory;

    SinkFactory *
    SinkFactory::s_instance = &s_flatsink_factory;

    // ----- LogState -----

    PpSink &
    LogState::sink()
    {
        if (!sink_) {
            this->builtin_flag_ = SinkFactory::instance().is_flat();
            this->sink_ = SinkFactory::instance().create();
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
