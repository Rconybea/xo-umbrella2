/** @file PpSinkFactory.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "PpSinkFactory.hpp"
#include "FlatSink.hpp"
#include <iostream>

namespace xo::pp {

    PpSinkFactory &
    PpSinkFactory::set_instance(PpSinkFactory * x)
    {
        s_instance = x;
        return *s_instance;
    }

    /** process-wide default sink: flat output to std::clog.
     *  (POC: whole-program FlatSink; per-thread interleaving not yet addressed)
     **/
    class FlatSinkFactory : public PpSinkFactory {
    public:
        static FlatSinkFactory & instance() { return s_instance; }

        /** fallback factory: creates flat sinks **/
        virtual bool is_flat() const override { return true; }
        /** create FlatSink instance **/
        virtual std::unique_ptr<PpSink> create(ColorSelect c, SinkOutput d) override;

    private:
        static FlatSinkFactory s_instance;
    };

    FlatSinkFactory
    FlatSinkFactory::s_instance;

    std::unique_ptr<PpSink>
    FlatSinkFactory::create(ColorSelect c, SinkOutput d)
    {
        PpStyle style;
        {
            switch (c) {
            case ColorSelect::k_plain:
                style = PpStyle::plain();
                break;
            case ColorSelect::k_colored:
                style = PpStyle::colored();
                break;
            }
        }

        std::streambuf * sbuf = nullptr;
        {
            switch (d) {
            case SinkOutput::k_memory:
                sbuf = nullptr;
                break;
            case SinkOutput::k_clog:
                sbuf = std::clog.rdbuf();
                break;
            }
        }

        /** low-dependency fallback. No pretty-printing **/
        return std::make_unique<FlatSink>(style, sbuf);
    }

    // ----- PpSinkFactory -----

    PpSinkFactory *
    PpSinkFactory::s_instance = &FlatSinkFactory::instance();

} /*namespace xo::pp*/

/* end PpSinkFactory.cpp */
