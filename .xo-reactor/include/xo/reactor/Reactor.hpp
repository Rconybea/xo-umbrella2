/* @file Reactor.hpp */

#pragma once

#include "xo/refcnt/Refcounted.hpp"
#include "xo/indentlog/log_level.hpp"
#include <cstdint>

namespace xo {
    namespace reactor {
        class ReactorSource;

        /* abtract api for a reactor:
         * something that arranges to have work done on a set of Sources.
         */
        class Reactor : public ref::Refcount {
        public:
            virtual ~Reactor() = default;

            log_level loglevel() const { return loglevel_; }
            void set_loglevel(log_level loglevel) { loglevel_ = loglevel; }

            /* add source src to this reactor.
             * on success, invoke src.notify_reactor_add(this)
             *
             * returns true if source added;  false if already present
             */
            virtual bool add_source(bp<ReactorSource> src) = 0;

            /* remove source src from this reactor.
             * source must previously have been added by
             * .add_source(src).
             *
             * on success, invoke src.notify_reactor_remove(this)
             *
             * returns true if source removed;  false if not present
             */
            virtual bool remove_source(bp<ReactorSource> src) = 0;

            /* notification when non-primed source (source with no known events)
             * becomes primed (source with at least one event)
             */
            virtual void notify_source_primed(bp<ReactorSource> src) = 0;

            /* dispatch one reactor event,  borrowing the calling thread
             * amount of work this represents is Source/Sink specific.
             *
             * returns #of events dispatched (0 or 1)
             */
            virtual std::uint64_t run_one() = 0;

            /* borrow calling thread to dispatch reactor events.
             * if n is -1,   run indefinitely
             * otherwise dispatch up to n events.
             * n = 0 is a noop
             */
            std::uint64_t run_n(int32_t n);

            /* borrow calling thread to run indefinitely.
             * suitable implementation for dedicated reactor threads
             */
            void run() { this->run_n(-1); }

            /** print self human-readably on stream @p os
             **/
            virtual void display(std::ostream & os) const = 0;

        protected:
            Reactor();

        private:
            /* control logging verbosity */
            log_level loglevel_;
        }; /*Reactor*/

        inline std::ostream &
        operator<<(std::ostream & os, const Reactor & x) {
            x.display(os);
            return os;
        }
    } /*namespace reactor*/
} /*namespace xo*/

/* end Reactor.hpp */
