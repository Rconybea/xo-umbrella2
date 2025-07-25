/* @file PollingReactor.hpp */

#pragma once

#include "Reactor.hpp"
#include "ReactorSource.hpp"
#include <vector>
#include <cstdint>

namespace xo {
    namespace reactor {
        /* reactor that runs by polling an ordered set of sources */
        class PollingReactor : public Reactor {
        public:
            /* named ctor idiom */
            static rp<PollingReactor> make() { return new PollingReactor(); }

            // ----- inherited from Reactor -----

            virtual bool add_source(bp<ReactorSource> src) override;
            virtual bool remove_source(bp<ReactorSource> src) override;
            virtual void notify_source_primed(bp<ReactorSource> src) override;
            virtual std::uint64_t run_one() override;

            virtual void display(std::ostream & os) const override;

        private:
            PollingReactor() = default;

            /* find non-empty source,  starting from .source_v_[start_ix],
             * wrapping around to .source_v_[start_ix - 1].
             *
             * return index of first available non-empty source,
             * or -1 if all sources are empty
             */
            std::int64_t find_nonempty_source(std::size_t start_ix);

        private:
            /* next source to poll will be .source_v_[.next_ix_] */
            std::size_t next_ix_ = 0;

            /* ordered set of sources (see reactor::Source)
             * reactor will poll sources in round-robin order
             */
            std::vector<ReactorSourcePtr> source_v_;
        }; /*PollingReactor*/

    } /*namespace reactor*/

#ifndef ppdetail_atomic
    namespace print {
        using PollingReactorPointer = xo::reactor::PollingReactor*;
        // placeholder, until we implement pretty-printing
        PPDETAIL_ATOMIC(xo::reactor::PollingReactor);
        PPDETAIL_ATOMIC(PollingReactorPointer);
    }
#endif
} /*namespace xo*/

/* end PollingReactor.hpp */
