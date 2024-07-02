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
            /** true if this is toplevel (global) environment.
             *  Toplevel environment doesn't have slot numbers.
             *
             *  Variables that bind in the global environment have unique
             *  names, which we rely on instead of slot numbers.
             **/
            virtual bool is_global_env() const = 0;

            /** lookup binding path for @p vname in this environment.
             *
             *  Reports ingredients needed to address variable at runtime,
             *  in runtime analog of this environment
             **/
            virtual binding_path lookup_binding(const std::string & vname) const = 0;

            /** lookup variable-expression @p vname in this environment.
             *  returns llvm::Value representing code that produces a value for vname
             **/
            virtual ref::brw<Expression> lookup_var(const std::string & vname) const = 0;
        };
    } /*namespace ast*/
} /*namespace xo*/


/* end Environment.hpp */
