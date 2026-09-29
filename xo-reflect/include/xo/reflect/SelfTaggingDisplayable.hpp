/* file SelfTaggingDisplayable.hpp
 *
 * author: Roland Conybeare, Sep 2026
 */

#pragma once

#include "TaggedRcptr.hpp"
#include "TypeDescr.hpp"
#include <xo/refcnt/Displayable.hpp>

namespace xo {
    namespace reflect {
        /* SelfTagging (see SelfTagging.hpp), for a class that is also
         * Displayable.  SelfTagging and ref::Displayable each inherit
         * ref::Refcount, and Refcount is not a virtual base, so a class
         * cannot inherit both: this one inherits Refcount by way of
         * Displayable instead.
         *
         * self_tp() reports the object's actual (most-derived) type -- e.g.
         * an implementation class behind an abstract interface.
         */
        class SelfTaggingDisplayable : public ref::Displayable {
        public:
            virtual TaggedRcptr self_tp() = 0;
        }; /*SelfTaggingDisplayable*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end SelfTaggingDisplayable.hpp */
