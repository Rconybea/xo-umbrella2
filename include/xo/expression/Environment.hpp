/* file Environment.hpp
 *
 * author: Roland Conybeare, Jun 2024
 */

#pragma once

#include "xo/refcnt/Refcounted.hpp"
#include "Variable.hpp"

namespace xo {
    namespace ast {
        class Environment : public ref::Refcount {
        public:
            /** lookup variable-expression @p vname in this environment
             **/
            virtual ref::brw<Variable> lookup_var(const std::string & vname) const = 0;
        };
    } /*namespace ast*/
} /*namespace xo*/


/* end Environment.hpp */
