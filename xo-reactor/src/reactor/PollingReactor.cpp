/* @file PollingReactor.cpp */

#include "PollingReactor.hpp"

namespace xo {
    using ref::brw;
    using std::size_t;
    using std::uint64_t;
    using std::int64_t;

    namespace reactor {
        bool
        PollingReactor::add_source(brw<ReactorSource> src)
        {
            /* make sure src does not already appear in .source_v[] */
            for(ReactorSourcePtr const & x : this->source_v_) {
                if(x.get() == src.get()) {
                    throw std::runtime_error("PollingReactor::add_source; source already present");
                    return false;
                }
            }

            src->notify_reactor_add(this);

            this->source_v_.push_back(src.get());

            return true;
        } /*add_source*/

        bool
        PollingReactor::remove_source(brw<ReactorSource> src)
        {
            auto ix = std::find(this->source_v_.begin(),
                                this->source_v_.end(),
                                src);

            if(ix != this->source_v_.end()) {
                src->notify_reactor_remove(this);

                this->source_v_.erase(ix);

                return true;
            }

            return false;
        } /*remove_source*/

        void
        PollingReactor::notify_source_primed(brw<ReactorSource>) {
            /* nothing to do here -- all sources always checked by polling loop */
        } /*notify_source_primed*/

        int64_t
        PollingReactor::find_nonempty_source(size_t start_ix)
        {
            size_t z = this->source_v_.size();

            /* search sources [ix .. z) */
            for(size_t ix = start_ix; ix < z; ++ix) {
                brw<ReactorSource> src = this->source_v_[ix];

                if(src->is_nonempty())
                    return ix;
            }

            /* search source [0 .. ix) */
            for(size_t ix = 0, n = std::min(start_ix, z); ix < n; ++ix) {
                brw<ReactorSource> src = this->source_v_[ix];

                if(src->is_nonempty())
                    return ix;
            }

            return -1;
        } /*find_nonempty_source*/

        uint64_t
        PollingReactor::run_one()
        {
            int64_t ix = this->find_nonempty_source(this->next_ix_);

            scope log(XO_DEBUG(this->loglevel() <= log_level::chatty));

            log && log(xtag("self", this), xtag("src_ix", ix));

            uint64_t retval = 0;

            if(ix >= 0) {
                brw<ReactorSource> src = this->source_v_[ix];

                log && log(xtag("src.name", src->name()));

                retval = src->deliver_one();
            } else {
                retval = 0;
            }

            log.end_scope(xtag("retval", retval));

            return retval;
        } /*run_one*/
    } /*namespace reactor*/
} /*namespace xo*/

/* end PollingReactor.cpp */
