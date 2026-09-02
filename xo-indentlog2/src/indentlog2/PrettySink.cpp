/** @file PrettySink.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PrettySink.hpp"
#include <xo/ppsink/LogState.hpp>
#include <iostream>
#include <cstdlib>

namespace xo {
    using xo::mm::ArenaConfig;

    namespace pp {
        std::unique_ptr<PpSink>
        PrettySinkFactory::create()
        {
            return std::make_unique<PrettySink>(pp_config_,
                                                std::clog.rdbuf());
        }

        PrettySink::PrettySink(const PpConfig & cfg,
                               std::streambuf * out)
        : PpSink(PpStyle::default_style()),
          pps_{cfg.layout()},
          sbuf_{&pps_},
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
        PrettySink::scratch_plain(std::string basename,
                                  uint32_t size,
                                  uint32_t margin)
        {
            return scratch_aux(basename, size, margin, PpStyle::plain());
        }

        PrettySink
        PrettySink::scratch_aux(std::string basename,
                                uint32_t size,
                                uint32_t margin,
                                const PpStyle & style)
        {
            return PrettySink(PpConfig::scratch_aux(basename,
                                                    margin,
                                                    style).with_logbuf_size(size), nullptr);
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
        PrettySink::begin_here(std::int32_t offset)
        {
            pps_.begin_here(offset);
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
#ifdef OBSOLETE
            os_.clear();  // in case badbit set on previous use
#endif

            return PpSinkInserter(this, &sbuf_);
        }

        void
        PrettySink::stream_commit()
        {
            sbuf_.commit();
        }

#ifdef OBSOLETE
        bool
        ThreadPrettySink::thread_install_once(const PpConfig & cfg, std::streambuf * out)
        {
            /** Default-constructed PpConfig has logbuf size 0,
             *  because policy is to force app to make a deliberate choice.
             *
             *  PrettySink requires a non-zero buffer size
             *  (simpler cousin FlatSink does not use logbuf)
             **/
            if (cfg.logbuf().logbuf_config().size_ == 0) {
                std::cerr
                    << "ThreadPrettySink::thread_install_once"
                    << ": logbuf arena has size 0 -- a PrettySink cannot buffer a record."
                    << "  Pass e.g. PpConfig().with_logbuf_config"
                    << "(ArenaConfig().with_size(1024*1024))"
                    << std::endl;

                std::abort();
            }

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
#endif

    } /*namespace pp*/
} /*namespace xo*/

/* end PrettySink.cpp */
