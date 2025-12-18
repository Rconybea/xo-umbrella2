/** @file IAllocIterator_DArenaIterator.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "arena/IAllocIterator_DArenaIterator.hpp"
#include "AllocIterator.hpp"
#include <cassert>

namespace xo {
    using std::byte;

    namespace mm {
        AllocInfo
        IAllocIterator_DArenaIterator::deref(const DArenaIterator & ix) noexcept
        {
            return ix.deref();
        }

        cmpresult
        IAllocIterator_DArenaIterator::compare(const DArenaIterator & ix,
                                               const obj<AAllocIterator> & other_arg) noexcept
        {
            /* downcast from variant */
            auto other = obj<AAllocIterator, DArenaIterator>::from(other_arg);

            if (!other)
                return cmpresult::incomparable();

            DArenaIterator & other_ix = *other;

            return ix.compare(other_ix);
        }

        void
        IAllocIterator_DArenaIterator::next(DArenaIterator & ix) noexcept
        {
            ix.next();
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocIterator_DArenaIterator.cpp */
