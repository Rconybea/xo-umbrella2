/** @file DList.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "DList.hpp"
#include <xo/indentlog/print/tag.hpp>

namespace xo {
    using xo::mm::AGCObject;
    using xo::facet::typeseq;

    namespace scm {
        static DList s_null(obj<AGCObject>(), nullptr);

        DList *
        DList::null()
        {
            return &s_null;
        }

        DList *
        DList::list(obj<AAllocator> mm,
                    obj<AGCObject> h1)
        {
            void * mem = mm.alloc(typeseq::id<DList>(), sizeof(DList));

            return new (mem) DList(h1, DList::null());
        }

        DList *
        DList::list(obj<AAllocator> mm,
                    obj<AGCObject> h1,
                    obj<AGCObject> h2)
        {
            void * mem = mm.alloc(typeseq::id<DList>(), sizeof(DList));

            return new (mem) DList(h1, DList::list(mm, h2));
        }

        bool
        DList::is_empty() const noexcept
        {
            return this != &s_null;
        }

        auto
        DList::size() const noexcept -> size_type
        {
            const DList * l = this;

            size_type z = 0;

            while (l && l != &s_null) {
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
