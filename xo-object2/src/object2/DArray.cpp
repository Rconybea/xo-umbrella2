/** @file DArray.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArray.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/print/pretty.hpp>
#include <xo/indentlog/print/tostr.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <cstdint>

namespace xo {
    using xo::print::APrintable;
    using xo::facet::FacetRegistry;
    using xo::mm::AGCObject;
    using xo::facet::typeseq;

    namespace scm {
        DArray *
        DArray::empty(obj<AAllocator> mm,
                      size_type cap)
        {
            DArray * result = nullptr;

            void * mem = mm.alloc(typeseq::id<DArray>(),
                                  sizeof(DArray) + cap * sizeof(obj<AGCObject>));

            result = new (mem) DArray();

            assert(result);

            result->capacity_ = cap;
            result->size_ = 0;

            return result;
        }

        DArray *
        DArray::copy(obj<AAllocator> mm,
                     DArray * src,
                     size_type new_cap)
        {
            DArray * dest = empty(mm, new_cap);

            /** could just memcpy here **/
            for (size_type i = 0, n = src->size(); i < n; ++i) {
                dest->elts_[i] = src->elts_[i];
            }

            dest->size_ = src->size();

            return dest;
        }

        obj<AGCObject>
        DArray::at(size_type ix) const
        {
            if (ix < size_) {
                return elts_[ix];
            } else {
                throw std::runtime_error(tostr("DArray::at: out-of-range index where [0..z) expected",
                                               xtag("index", ix),
                                               xtag("z", this->size())));
                return obj<AGCObject>();
            }
        }

        bool
        DArray::assign_at(size_type ix, obj<AGCObject> x) noexcept
        {
            if (ix >= size_)
                return false;

            scope log(XO_DEBUG(true), "need write barrier");

            this->elts_[ix] = x;

            return true;
        }

        bool
        DArray::push_back(obj<AGCObject> elt) noexcept
        {
            if (size_ >= capacity_) {
                return false;
            } else {
                static_assert(!std::is_trivially_constructible_v<obj<AGCObject>>);

                void * mem = &(elts_[size_]);

                new (mem) obj<AGCObject>(elt);

                ++(this->size_);

                return true;
            }
        }

        bool
        DArray::resize(size_type new_z) noexcept
        {
            if (new_z > capacity_) {
                return false;
            } else if (new_z > size_) {
                // ensure new size is zeroed (we/re not zeroing if/when we shrink)
                ::memset((std::byte *)(&elts_[size_]), 0, (std::byte *)(&elts_[new_z]) - (std::byte *)(&elts_[size_]));
            }

            this->size_ = new_z;
            return true;
        }

        void
        DArray::shrink_to_fit() noexcept
        {
            if (capacity_ > size_) {
                this->capacity_ = size_;
            }
        }

        // printing support

        bool
        DArray::pretty(const ppindentinfo & ppii) const
        {
            using xo::print::ppstate;

            ppstate * pps = ppii.pps();

            if (ppii.upto()) {
                /* perhaps print on one line */
                pps->write("[");

                for (size_t i = 0, n = this->size(); i < n; ++i ) {
                    if (i > 0)
                        pps->write(" ");

                    obj<APrintable> elt
                        = FacetRegistry::instance().variant<APrintable,AGCObject>(this->at(i));

                    assert(elt.data());

                    if (!pps->print_upto(elt))
                        return false;
                }

                pps->write("]");
                return true;
            } else {
                pps->write("[");

                for (size_type i = 0, n = this->size(); i < n; ++i) {
                    if (i == 0) {
                        /* indent, but credit initial [ */
                        pps->indent(std::max(pps->indent_width(), 1u) - 1);
                    } else {
                        /* indent after newline */
                        pps->newline_indent(ppii.ci1());
                    }

                    obj<APrintable> elt = this->at(i).to_facet<APrintable>();

                    assert(elt.data());

                    pps->pretty(elt);
                }

                pps->write("]");
                return false;
            }
        }

        // gc hooks for IGCObject_DArray

        std::size_t
        DArray::shallow_size() const noexcept
        {
            return sizeof(DArray) + (capacity_ * sizeof(obj<AGCObject>));
        }

        DArray *
        DArray::shallow_move(obj<AAllocator> mm) const noexcept
        {
            DArray * copy = (DArray *)mm.alloc_copy((std::byte *)this);

            if (copy) {
                copy->capacity_ = capacity_;
                copy->size_ = size_;

                constexpr auto c_obj_z = sizeof(obj<AGCObject>);

                /* memcpy sufficient for obj<A,D> */
                ::memcpy((void*)&(copy->elts_[0]),
                         (void*)&(elts_[0]),
                         capacity_ * c_obj_z);
            }

            return copy;
        }

        std::size_t
        DArray::forward_children(obj<ACollector> gc) noexcept
        {
            scope log(XO_DEBUG(false));

            for (size_type i = 0; i < size_; ++i) {
                log && log("DArray::forward_children (loop)", xtag("i", i), xtag("z", size_));

                obj<AGCObject> & elt = elts_[i];

                gc.forward_inplace(&elt);
            }

            return shallow_size();
        }

    } /*namespace scm*/
} /*namespace xo*/
