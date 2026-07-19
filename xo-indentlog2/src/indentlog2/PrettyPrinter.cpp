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
          sbuf_{&pps_},
          os_(&sbuf_),
          logbuf_{cfg.logbuf_config(), cfg.logbuf_debug_flag()}
        {
            pps_.connect_output(&logbuf_);
        }

        void
        PrettyPrinter::visit_pools(const MemorySizeVisitor & fn) const
        {
            pps_.visit_pools(fn);
            logbuf_.visit_pools(fn);
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

        PpSinkInserter
        PrettyPrinter::stream_open(uint32_t min_z)
        {
            assert(pps_.has_open_string() == false);

            sbuf_.open(min_z);
            os_.clear();  // in case badbit set on previous use

            return PpSinkInserter(this, &os_);
        }

        void
        PrettyPrinter::stream_commit()
        {
            sbuf_.commit();
        }

    } /*namespace print*/
} /*namespace xo*/

/* end PrettyPrinter.cpp */
