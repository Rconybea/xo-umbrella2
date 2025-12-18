/** @file DX1CollectorIterator.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "gc/DX1CollectorIterator.hpp"
#include "gc/DX1Collector.hpp"
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>

namespace xo {
    namespace mm {
        DX1CollectorIterator::DX1CollectorIterator(const DX1Collector * gc,
                                                   generation gen_ix,
                                                   generation gen_hi,
                                                   DArenaIterator arena_ix,
                                                   DArenaIterator arena_hi) : gc_{gc},
                                                                              gen_ix_{gen_ix},
                                                                              gen_hi_{gen_hi},
                                                                              arena_ix_{arena_ix},
                                                                              arena_hi_{arena_hi}
        {
            this->normalize();
        }

        void
        DX1CollectorIterator::normalize() noexcept
        {
            /* normalize: find lowest generation with non-empty to-space */

            for (; gen_ix_ < gen_hi_; ++gen_ix_) {
                const DArena * arena
                    = gc_->get_space(role::to_space(), gen_ix_);

                arena_ix_ = arena->begin();
                arena_hi_ = arena->end();

                if (arena_ix_ != arena_hi_) {
                    // normalization achieved!
                    break;
                }
            }
        }

        AllocInfo
        DX1CollectorIterator::deref() const noexcept
        {
            return arena_ix_.deref();
        }

        cmpresult
        DX1CollectorIterator::compare(const DX1CollectorIterator & other_ix) const noexcept
        {
            scope log(XO_DEBUG(false),
                      xtag("is_valid", is_valid()),
                      xtag("other_ix.is_valid", other_ix.is_valid()) );

            if (is_invalid() || (gc_ != other_ix.gc_)) {
                log && log("incomparable!");
                return cmpresult::incomparable();
            }

            if (gen_ix_ != other_ix.gen_ix_) {
                log && log(xtag("gen", gen_ix_), xtag("other.gen", other_ix.gen_ix_));

                /* same collector, different arenas -> compare based on gen# */

                return cmpresult::from_cmp(gen_ix_, other_ix.gen_ix_);
            }

            /* both iterators refer to the same arena,
             * so can compare their arena iterators directly
             */
            cmpresult retval = arena_ix_.compare(other_ix.arena_ix_);

            log && log(xtag("retval", retval));

            return retval;
        }

        void
        DX1CollectorIterator::next() noexcept
        {
            if (arena_ix_ != arena_hi_) {
                ++arena_ix_;
                this->normalize();
            }
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end DX1CollectorIterator.cpp */
