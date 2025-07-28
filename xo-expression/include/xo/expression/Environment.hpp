/* file Environment.hpp
 *
 * author: Roland Conybeare, Jun 2024
 */

#pragma once

#include "xo/refcnt/Refcounted.hpp"
#include "Variable.hpp"
#include "binding_path.hpp"
#include "xo/indentlog/print/pretty.hpp"

namespace xo {

    namespace scm {
        class Expression;

        /** @class Environment
         *  @brief Abstract interface for tracking variable bindings
         *
         *  When parsing (see xo-reader): rhs will always be a variable.
         *  When generating code (see xo-jit): rhs can be any expression,
         *  for example a Lambda.
         **/
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
            virtual bp<Expression> lookup_var(const std::string & vname) const = 0;

            /** like @ref lookup_var but do not delegate to parent environment **/
            virtual bp<Expression> lookup_local(const std::string & vname) const = 0;

            /** create/replace local variable @p target.
             *  Narrow use case: intended for when Environment represents a top-level session environment
             **/
            virtual void upsert_local(bp<Variable> target) = 0;

            virtual void print(std::ostream & os) const = 0;
            virtual std::uint32_t pretty_print(const xo::print::ppindentinfo & ppii) const = 0;
        };

        inline std::ostream &
        operator<< (std::ostream & os, const Environment & x) {
            x.print(os);
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end Environment.hpp */
