/** @file PrettyPrinter.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PrettyPrinter.hpp"
//#include "print/PpToken.hpp"
//#include <cassert>
//#include <cstring>

namespace xo {
    //using xo::mm::ArenaConfig;
    //using std::max;
    //using std::min;

    namespace print {
        PrettyPrinter::PrettyPrinter(const PpConfig & cfg)
          : pps_{cfg},
            logbuf_{cfg.logbuf_config(), cfg.logbuf_debug_flag()}
        {
            pps_.connect_output(&logbuf_);
        }

        void
        PrettyPrinter::put(std::string_view x)
        {
            pps_.put(x);
        }

        void
        PrettyPrinter::begin()
        {
            pps_.begin();
        }

        void
        PrettyPrinter::split()
        {
            pps_.split();
        }

        void
        PrettyPrinter::end()
        {
            pps_.end();
        }

    } /*namespace print*/
} /*namespace xo*/

/* end PrettyPrinter.cpp */
