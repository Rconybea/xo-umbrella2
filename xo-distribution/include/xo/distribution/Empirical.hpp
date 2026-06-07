/* @file Empirical.hpp */

#pragma once

#include "xo/distribution/Distribution.hpp"
#include "xo/ordinaltree/RedBlackTree.hpp"
#include "xo/indentlog/scope.hpp"
#include <map>
#include <cstdint>

namespace xo {
    namespace distribution {

        /* representation for counter,
         * recording #of samples with the same value
         */
        using CounterRep = uint32_t;

        /* counter;  for use with StdEmpirical distribution below
         */
        class Counter {
        public:
            Counter() = default;
            Counter(CounterRep n) : count_(n) {}

            CounterRep count() const { return count_; }

            void incr() { ++count_; }

            operator CounterRep () const { return count_; }

            Counter & operator+=(CounterRep n) { count_ += n; return *this; }

        private:
            CounterRep count_ = 0;
        }; /*Counter*/

    } /*namespace disitribution*/
} /*namespace xo*/

/* end Empirical.hpp */
