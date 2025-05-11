/* @file Distribution.hpp */

#pragma once

#include "xo/refcnt/Refcounted.hpp"

namespace xo {
    namespace distribution {
        /* abstract api for a cumulative probability distribution.
         * over supplied Domain
         */
        template<typename Domain>
        class Distribution : public ref::Refcount {
        public:
            virtual double cdf(Domain const & x) const = 0;
        }; /*Distribution*/
    } /*namespace distribution*/
} /*namespace xo*/

/* end Distribution.hpp */
