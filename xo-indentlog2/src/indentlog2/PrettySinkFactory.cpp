/** @file PrettySinkFactory.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "print/PrettySinkFactory.hpp"
#include "print/PrettySink.hpp"
#include <iostream>

namespace xo::pp {

    std::unique_ptr<PpSink>
    PrettySinkFactory::create(ColorSelect c, SinkOutput d)
    {
        PpStyle style;
        {
            switch (c) {
            case ColorSelect::k_plain:
                style = PpStyle::plain();
                break;
            case ColorSelect::k_colored:
                style = pp_config_.style();
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

        return std::make_unique<PrettySink>(pp_config_.with_style(style),
                                            sbuf);
    }

} /*namespace xo::pp*/

/* end PrettySinkFactory.cpp */
