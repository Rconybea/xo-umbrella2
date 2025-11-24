/** @file Env.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/alloc/Object.hpp"
#include "xo/refcnt/Refcounted.hpp"

namespace xo {
    namespace scm {
        class Variable; // see xo::scm::Variable in xo/expression/Variable.hpp

        /** @class Env
         *  @brief runtime environment, holding variable bindings for schematika interpreter
         *
         *  Garbage-collected
         *
         * TODO: rename xo-expression xo::scm::Environment -> xo::scm::SymbolTable
         **/
        class Env : public Object {
        public:
            /** true iff @p vname is present in Symtab for innermost environment **/
            virtual bool local_contains_var(const std::string & vname) const = 0;

            /** require storage for variable @p v.
             *  will also establish binding path.
             *
             *  Intended for introducing a new variable,
             *  replacing any previous variable with the same name.
             *
             *  Beware of invalidating type correctness
             **/
            virtual void establish_var(bp<Variable> v) = 0;

            //gp<Object> lookup_symbol(const std::string & name) const;
        };
    } /*namespace scm*/
} /*namespace xo*/
