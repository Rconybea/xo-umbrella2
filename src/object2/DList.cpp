/** @file DList.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "DList.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/print/pretty.hpp>
#include <xo/indentlog/print/tag.hpp>

namespace xo {
    using xo::print::APrintable;
    using xo::mm::AGCObject;
    using xo::facet::FacetRegistry;
    using xo::facet::typeseq;

    namespace scm {
        static DList s_null(obj<AGCObject>(), nullptr);

        DList *
        DList::_nil()
        {
            return &s_null;
        }

        DList *
        DList::_cons(obj<AAllocator> mm,
                     obj<AGCObject> car,
                     DList * cdr)
        {
            void * mem = mm.alloc(typeseq::id<DList>(), sizeof(DList));

            return new (mem) DList(car, cdr);
        }

#ifdef OBSOLETE
        DList *
        DList::list(obj<AAllocator> mm,
                    obj<AGCObject> h1)
        {
            void * mem = mm.alloc(typeseq::id<DList>(), sizeof(DList));

            return new (mem) DList(h1, DList::_nil());
        }

        DList *
        DList::list(obj<AAllocator> mm,
                    obj<AGCObject> h1,
                    obj<AGCObject> h2)
        {
            void * mem = mm.alloc(typeseq::id<DList>(), sizeof(DList));

            return new (mem) DList(h1, DList::list(mm, h2));
        }
#endif

        bool
        DList::is_empty() const noexcept
        {
            return this == &s_null;
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

        bool
        DList::pretty(const ppindentinfo & ppii) const
        {
            /* adapted from ppstate.pretty_struct(), see also */

            using xo::print::ppstate;

            ppstate * pps = ppii.pps();

            if (ppii.upto()) {
                /* perhaps print on one line */
                pps->write("(");

                /* TODO: probably use iterators here, when available */
                const DList * l = this;

                size_t i = 0;
                while (!l->is_empty()) {
                    if (i > 0)
                        pps->write(" ");

                    obj<APrintable> elt
                        = FacetRegistry::instance().variant<APrintable, AGCObject>(l->head_);

                    assert(elt);

                    if (!pps->print_upto(elt))
                        return false;

                    l = l->rest_;
                    ++i;
                }

                pps->write(")");
                return true;
            } else {
                pps->write("(...)");
                return false;
            }
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DList.cpp */
