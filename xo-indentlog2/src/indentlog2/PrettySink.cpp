/** @file PrettySink.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PrettySink.hpp"
#include <xo/ppsink/LogState.hpp>
#include <iostream>

namespace xo {
    using xo::mm::ArenaConfig;

    namespace pp {
        PrettySink::PrettySink(const PpConfig & cfg,
                               std::streambuf * out)
        : PpSink(PpStyle::default_style()),
          pps_{cfg.layout()},
          sbuf_{&pps_},
          os_(&sbuf_),
          logbuf_{cfg.logbuf().logbuf_config(), cfg.logbuf().logbuf_debug_flag()}
        {
            /* presentation style travels with the config (PpConfig::style()),
             * but is CONSUMED through PpSink::style() -- the Prettifiers that
             * read it are handed only a PpSink.  See PpStyle.hpp.
             */
            this->style_ = cfg.style();

            /* collect pretty output in .logbuf_.. */
            pps_.connect_output(&logbuf_);

            /* ..and flush to out */
            logbuf_.set_dest_sbuf(out);
        }

        PrettySink
        PrettySink::scratch(std::string basename,
                            uint32_t size,
                            uint32_t margin)
        {
            static int s_seq = 0;

            ArenaConfig logbuf_cfg { .name_ = basename + std::to_string(++s_seq),
                                     .size_ = size };

            PpConfig cfg
                = PpConfig()
                   .with_logbuf_config(logbuf_cfg)
                   .with_soft_right_margin(margin)
                   .with_style(PpStyle::plain());

            return PrettySink(cfg, nullptr);
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
        PrettySink::complete()
        {
            /* Terminating newline at top level: check_print_ready() flushes the
             * whole record into logbuf_.  reset_buffer() then drains it to the
             * attached streambuf and reclaims the buffer.
             */
            pps_.put("\n");
            logbuf_.reset_buffer();
            return *this;
        }

        void
        PrettySink::set_dest_sbuf(std::streambuf * out)
        {
            logbuf_.set_dest_sbuf(out);
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

        bool
        ThreadPrettySink::thread_install_once(const PpConfig & cfg, std::streambuf * out)
        {
            if (ThreadLogState::thread_log_state().is_builtin_default()) {
                // first call with explicit sink

                static thread_local PrettySink * s_pretty_sink = nullptr;

                if (!s_pretty_sink) {
                    s_pretty_sink = new PrettySink(cfg, out);

                    ThreadLogState::log_set_sink(s_pretty_sink);

                    return true;
                }
            }

            return false;
        }

    } /*namespace pp*/
} /*namespace xo*/

/* end PrettySink.cpp */
