/** @file lambda_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"
//#include <cstdint>

namespace xo {
    namespace scm {
        /** @class lambda_xs
         *  @brief parsing state-machine for a lambda-expression
         **/
        class lambda_xs : public exprstate {
        public:
            lambda_xs();

        private:
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end lambda_xs.hpp **/
