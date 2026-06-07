/* @file Displayable.hpp */

#pragma once

#include "Refcounted.hpp"

namespace xo {
    namespace ref {
        class Displayable : public Refcount {
        public:
            /* write some kind of human-readable representation on stream */
            virtual void display(std::ostream & os) const = 0;
            std::string display_string() const;
        }; /*Displayable*/

        /* see also
         *   operator<<(std::ostream &, intrusive_ptr<T> const &)
         * in [Refcounted.hpp]
         */
        inline std::ostream &
        operator<<(std::ostream &os, Displayable const & x) {
            x.display(os);
            return os;
        } /*operator<<*/

    } /*namespace ref*/
} /*namespace xo*/

/* end Displayable.hpp */
