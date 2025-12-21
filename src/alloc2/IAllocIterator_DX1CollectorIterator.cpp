/** @file IAllocIterator_DX1CollectorIterator.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "gc/IAllocIterator_DX1CollectorIterator.hpp"
#include "AllocIterator.hpp"
//#include <cassert>

namespace xo {
    namespace mm {
        AllocInfo
        IAllocIterator_DX1CollectorIterator::deref(const DX1CollectorIterator & ix) noexcept
        {
            return ix.deref();
        }

        cmpresult
        IAllocIterator_DX1CollectorIterator::compare(const DX1CollectorIterator & ix,
                                                     const obj<AAllocIterator> & other_arg) noexcept
        {
            /* downcast from variant */
            auto other = obj<AAllocIterator, DX1CollectorIterator>::from(other_arg);

            if (!other)
                return cmpresult::incomparable();

            DX1CollectorIterator & other_ix = *other.data();

            return ix.compare(other_ix);
        }

        void
        IAllocIterator_DX1CollectorIterator::next(DX1CollectorIterator & ix) noexcept
        {
            ix.next();
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocIterator_DX1CollectorIterator.cpp */
