/** @file DString.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DString.hpp"

namespace xo {
    using xo::facet::typeseq;

    namespace scm {
        DString *
        DString::empty(obj<AAllocator> mm,
                       size_type cap)
        {
            assert(cap > 0);

            DString * result = nullptr;

            if (cap > 0) {
                void * mem = mm.alloc(typeseq::id<DString>(),
                                      sizeof(DString) + cap);

                result = new (mem) DString();
                result->capacity_ = cap;
                result->size_ = 0;
                if (cap > 0) {
                    result->chars_[0] = '\0';
                }
            }

            return result;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end DString.cpp */
