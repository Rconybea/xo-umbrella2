/** @file PrettySink.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PrettySink.hpp"
//#include "print/PpToken.hpp"
//#include <cassert>
//#include <cstring>

namespace xo {
    //using xo::mm::ArenaConfig;
    //using std::max;
    //using std::min;

    namespace print {
        PrettySink::PrettySink(const PpConfig & cfg)
        : pps_{cfg},
          sbuf_{&pps_},
          os_(&sbuf_),
          logbuf_{cfg.logbuf_config(), cfg.logbuf_debug_flag()}
        {
            pps_.connect_output(&logbuf_);
        }

        void
        PrettySink::visit_pools(const MemorySizeVisitor & fn) const
        {
            pps_.visit_pools(fn);
            logbuf_.visit_pools(fn);
        }

        void
        PrettySink::put(std::string_view x)
        {
            pps_.put(x);
        }

        void
        PrettySink::begin()
        {
            pps_.begin();
        }

        void
        PrettySink::split()
        {
            pps_.split();
        }

        void
        PrettySink::end()
        {
            pps_.end();
        }

        PpSinkInserter
        PrettySink::stream_open(uint32_t min_z)
        {
            assert(pps_.has_open_string() == false);

            sbuf_.open(min_z);
            os_.clear();  // in case badbit set on previous use

            return PpSinkInserter(this, &os_);
        }

        void
        PrettySink::stream_commit()
        {
            sbuf_.commit();
        }

    } /*namespace print*/
} /*namespace xo*/

/* end PrettySink.cpp */
