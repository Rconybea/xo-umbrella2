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
            PrettySink(const PpConfig & cfg);

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
            virtual PpSink & begin() override final;
            virtual PpSink & begin(std::int32_t offset) override final;
            virtual PpSink & split(std::uint32_t spaces, std::int32_t offset) override final;
            virtual PpSink & newline(std::int32_t offset) override final;
            virtual PpSink & end() override final;
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
    } /*namespace pp*/
} /*namespace xo*/

/* end PrettySink.hpp */
