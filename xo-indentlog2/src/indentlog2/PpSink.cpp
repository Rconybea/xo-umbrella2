/** @file PpSink.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PpSink.hpp"

namespace xo::print {
    PpSinkInserter::PpSinkInserter(PpSink * ppsink, std::ostream * os)
      : ppsink_{ppsink}, os_{os}
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
            os_ = nullptr;
        }
    }

}

/* end PpSink.cpp */
