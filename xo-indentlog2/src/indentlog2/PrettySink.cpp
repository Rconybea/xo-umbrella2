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

    namespace pp {
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

        PpSink &
        PrettySink::put(std::string_view x)
        {
            pps_.put(x);
            return *this;
        }

        PpSink &
        PrettySink::put_with_escape(std::string_view x, bool quote_flag)
        {
            pps_.put_with_escape(x, quote_flag);
            return *this;
        }

        PpSink &
        PrettySink::begin()
        {
            pps_.begin();
            return *this;
        }

        PpSink &
        PrettySink::begin(std::int32_t offset)
        {
            pps_.begin(offset);
            return *this;
        }

        PpSink &
        PrettySink::split(std::uint32_t spaces, std::int32_t offset)
        {
            pps_.split(spaces, offset);
            return *this;
        }

        PpSink &
        PrettySink::newline(std::int32_t offset)
        {
            pps_.newline(offset);
            return *this;
        }

        PpSink &
        PrettySink::end()
        {
            pps_.end();
            return *this;
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

    } /*namespace pp*/
} /*namespace xo*/

/* end PrettySink.cpp */
