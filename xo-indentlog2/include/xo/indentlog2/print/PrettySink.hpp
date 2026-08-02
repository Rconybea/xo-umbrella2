/** @file PrettySink.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include <xo/ppsink/PpSink.hpp>
#include "PpTokenStreambuf.hpp"
#include "PpState.hpp"

namespace xo {
    namespace pp {
        /**
         *  Use:
         *  @code
         *    PpConfig ppc;
         *    PrettySink pp(ppc);
         *
         *  @endcode
         **/
        class PrettySink : public PpSink {
        public:
            using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

        public:
            PrettySink(const PpConfig & cfg, std::streambuf * out);

            /** attach (or detach, with nullptr) a streambuf that completed
             *  records are drained to (e.g. @c std::clog.rdbuf()).
             *  Forwards to the underlying @ref logbuf_.
             **/
            void set_dest_sbuf(std::streambuf * sb);

            LogBuffer & logbuf() { return logbuf_; }

            /** pretty-printed output flushed to @ref logbuf_ so far.
             *  Excludes content still buffered in @ref pps_.
             *  Valid until the next write or logbuf reset.
             **/
            std::string_view output() const {
                auto s = logbuf_.used_span();
                return std::string_view(s.lo(), s.hi());
            }

            /** visit mapped storage pools **/
            void visit_pools(const MemorySizeVisitor & fn) const;

            // inherited from PpSink

            /* keep the inherited split()/split(spaces) and newline()
             * convenience overloads visible alongside the overrides below
             */
            using PpSink::split;
            using PpSink::newline;

            virtual PpSink & put(std::string_view x) override final;
            virtual PpSink & put_with_escape(std::string_view x, bool quote_flag) override final;
            virtual PpSink & begin() override final;
            virtual PpSink & begin(std::int32_t offset) override final;
            virtual PpSink & split(std::uint32_t spaces, std::int32_t offset) override final;
            virtual PpSink & newline(std::int32_t offset) override final;
            virtual PpSink & end() override final;
            /** record boundary: emit the terminating newline, then drain the
             *  completed record to the attached streambuf and reclaim logbuf_.
             **/
            virtual PpSink & complete() override final;
            /** current visible output column (enables right-aligned fields) **/
            virtual std::optional<std::size_t> lpos() const override final {
                return logbuf_.viz_lpos();
            }
            virtual PpSinkInserter stream_open(uint32_t min_z) override final;
            virtual void stream_commit() override;

        private:
            /** Pretty printer state (+ driver) **/
            PpState pps_;

            /** stream buffer for writing into @ref pps_ (at the scan endpoint) **/
            PpTokenStreambuf sbuf_;
            /** output stream bound to sbuf_; (for writing at scan endpoint) **/
            std::ostream os_;

            /** Buffer for pretty-printed output **/
            LogBuffer logbuf_;
        };

        class ThreadPrettySink {
        public:
            /** install PrettySink for the calling thread.
             *  Modifies @code ThreadLogState::thread_log_state().sink() @endcode,
             *  the first time it is called.  Noop on subsequent calls.
             *
             *  @return true iff set output stream; false otherwise.
             **/
            static bool thread_install_once(const PpConfig & cfg,
                                            std::streambuf * out);

        };
    } /*namespace pp*/
} /*namespace xo*/

/* end PrettySink.hpp */
