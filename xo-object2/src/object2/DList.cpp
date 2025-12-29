/** @file DList.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "DList.hpp"
#include <xo/indentlog/print/tag.hpp>

namespace xo {
    namespace scm {
        auto
        DList::size() const noexcept -> size_type
        {
            const DList * l = this;

            size_type z = 0;

            while (l) {
                ++z;
                l = l->rest_;
            }

            return z;
        }

        auto
        DList::at(size_type index) const -> obj<AGCObject>
        {
            size_type ix = index;
            const DList * l = this;

            while (l->rest_ && (ix > 0)) {
                --ix;
                l = l->rest_;
            }

            if (ix > 0) {
                assert(l == nullptr);

                throw std::runtime_error
                    (tostr("DList::at: out-of-range index where [0..z) expected",
                           xtag("index", index),
                           xtag("z", this->size())));
            }

            assert(l);

            return l->head_;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DList.cpp */
