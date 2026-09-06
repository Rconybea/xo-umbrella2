/** @file PrettySink.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpState.hpp"
#include "PpTokenStreambuf.hpp"
#include <xo/ppsink/PpSink.hpp>

namespace xo {
    namespace pp {
        /** @brief factory that creates pretty-printing sinks.
         *
         *  Would expect one per thread.
         **/
        class PrettySinkFactory : public xo::pp::SinkFactory {
        public:
            explicit PrettySinkFactory(const PpConfig & x) : pp_config_{x} {}

            const PpConfig & pp_config() const { return pp_config_; }

            virtual bool is_flat() const override { return false; }
            /** create pretty-printing instance **/
            virtual std::unique_ptr<PpSink> create() override;

        private:
            /** pretty-printing configuration **/
            PpConfig pp_config_;
        };

        /**
         *  Use:
         *  @code
         *    PpConfig ppc;
         *    PrettySink pp(ppc, nullptr);
         *
         *  @endcode
         **/
        class PrettySink : public PpSink {
        public:
            using MemorySizeVisitor = xo::mm::MemorySizeVisitor;
            using uint32_t = std::uint32_t;

        public:
            /** @p cfg.  pretty-printing configuration.
             *  @p out.  if non-null, forward output to this streambuf.
             **/
            PrettySink(const PpConfig & cfg, std::streambuf * out);

            /** move ctor.
             *
             *  Hand-written because PrettySink holds two INTERIOR pointers --
             *  sbuf_ refers to pps_, and pps_ refers to logbuf_ -- so the
             *  implicit memberwise move leaves both aimed at the moved-from
             *  object.  That failed silently: a moved sink rendered EMPTY, or
             *  crashed in LogBufferAdapter::write_span.  Nothing in c++ moved
             *  one (make2str's return is elided), so it went unnoticed until a
             *  pybind11 binding moved it.  See PrettySink_move.test.cpp.
             *
             *  Everything else survives untouched: the remaining interior
             *  pointers (current_open_string_, the logbuf char pointers, the
             *  streambuf put area) all address arena memory, and DArena's move
             *  transfers the mapping without relocating it.
             *
             *  MAINTENANCE: adding a member that points at a sibling means
             *  repairing it here too.
             **/
            PrettySink(PrettySink && rhs) noexcept;

            /** no move assignment: nothing needs it, and it would have to
             *  tear down the existing arenas first
             **/
            PrettySink & operator=(PrettySink &&) = delete;

            /** create pretty sink that writes to string **/
            static PrettySink make2str(const PpConfig & cfg);

            /** create pretty sink that writes to cout **/
            static PrettySink make2cout(const PpConfig & cfg);

            /** create temporary pretty sink, intended for a unit test **/
            static PrettySink scratch_plain(std::string basename,
                                            uint32_t logbuf_z,
                                            uint32_t margin);
            /** create temporary pretty sink **/
            static PrettySink scratch_aux(std::string basename,
                                          uint32_t logbuf_z,
                                          uint32_t margin,
                                          const PpStyle & style);

            /** verify this sink's internal invariants: that sbuf_ still writes
             *  into OUR pps_, that pps_ still drains into OUR logbuf_, and
             *  whatever logbuf_ checks of its own.
             *
             *  Open to any code -- assert(verify_ok()) in a mutator, or call
             *  it directly from a test.  Throws std::runtime_error naming the
             *  broken invariant when @p throw_flag, else returns false.
             *
             *  The first two are the move-ctor repairs.  They are checked
             *  rather than left to crash because a stale interior pointer is
             *  undefined behaviour that in practice often does NOT fault
             *  in-process.  See PrettySink_move.test.cpp.
             **/
            bool verify_ok(bool throw_flag = true) const;

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
            virtual PpSink & begin_here(std::int32_t offset) override final;
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

            /** Buffer for pretty-printed output **/
            LogBuffer logbuf_;
        };

#ifdef OBSOLETE
        /** DEPRECATED.  Prefer Indentlog2_Appcx mechanism
         *
         *  See alloc2_utest_main.cpp, facet_utest_main.cpp, skreplxx.cpp
         **/
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
#endif
    } /*namespace pp*/
} /*namespace xo*/

/* end PrettySink.hpp */
