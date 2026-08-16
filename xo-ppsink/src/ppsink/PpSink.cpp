/** @file PpSink.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include <xo/ppsink/PpSink.hpp>

namespace xo::pp {
    PpSink::PpSink(const PpStyle & style)
      : style_{style}
    {}

    PpSinkInserter::PpSinkInserter(PpSink * ppsink, std::streambuf * sbuf)
      : ppsink_{ppsink}, sbuf_{sbuf}
    {
    }

    PpSinkInserter::~PpSinkInserter()
    {
        this->finish();
    }

    void
    PpSinkInserter::finish()
    {
        if (ppsink_) {
            ppsink_->stream_commit();

            // ensure we can't reuse this
            ppsink_ = nullptr;
            sbuf_ = nullptr;
        }
    }

}

/* end PpSink.cpp */
