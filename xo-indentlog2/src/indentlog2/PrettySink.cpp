/** @file PrettySink.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PrettySink.hpp"
#include <xo/ppsink/LogState.hpp>
#include <xo/ppsink/tostr0.hpp>
#include <xo/ppsink/tag.hpp>
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <cstdlib>

namespace xo {
    using xo::mm::ArenaConfig;
    using std::cout;

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

        PrettySink::PrettySink(PrettySink && rhs) noexcept
            : PpSink(std::move(rhs)),
              pps_{std::move(rhs.pps_)},
              sbuf_{std::move(rhs.sbuf_)},
              logbuf_{std::move(rhs.logbuf_)}
        {
            /* repair the two interior pointers: after the memberwise move both
             * still refer to rhs's members.  See the header for why nothing
             * else needs fixing.
             */
            sbuf_.reset_pps(&pps_);
            pps_.connect_output(&logbuf_);

            assert(this->verify_ok(false /*!throw_flag*/));
        }

        bool
        PrettySink::verify_ok(bool throw_flag) const
        {
            using xo::pp::tostr0;
            using xo::pp::xtag;

            /* 1. sbuf_ writes into OUR pps_ (move-ctor repair #1) */
            if (sbuf_._pps() != &pps_) {
                if (throw_flag) {
                    throw std::runtime_error
                        (tostr0("PrettySink::verify_ok",
                                ": sbuf_ does not address this sink's pps_"
                                " (missed a move-ctor repair?)",
                                xtag("sbuf.pps", (const void *)sbuf_._pps()),
                                xtag("expected", (const void *)&pps_)));
                }
                return false;
            }

            /* 2. pps_ drains into OUR logbuf_ (move-ctor repair #2) */
            if (pps_._out() != &logbuf_) {
                if (throw_flag) {
                    throw std::runtime_error
                        (tostr0("PrettySink::verify_ok",
                                ": pps_ does not drain into this sink's logbuf_"
                                " (missed a move-ctor repair?)",
                                xtag("pps.out", (const void *)pps_._out()),
                                xtag("expected", (const void *)&logbuf_)));
                }
                return false;
            }

            /* 3. and whatever logbuf_ checks of its own -- notably that ITS
             *    interior pointer addresses its own arena
             */
            return logbuf_.verify_ok(throw_flag);
        }

        PrettySink
        PrettySink::make2str(const PpConfig & cfg)
        {
            return PrettySink(cfg, nullptr);
        }

        PrettySink
        PrettySink::make2cout(const PpConfig & cfg)
        {
            return PrettySink(cfg, cout.rdbuf());
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
