/** @file Env.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/alloc/Object.hpp"

namespace xo {
    namespace scm {
        /** @class Env
         *  @brief runtime environment, holding variable bindings for schematika interpreter
         *
         *  Garbage-collected
         *
         * TODO: rename xo-expression xo::scm::Environment -> xo::scm::SymbolTable
         **/
        class Env : public Object {
        public:
            //gp<Object> lookup_symbol(xxx);
        };
    } /*namespace scm*/
} /*namespace xo*/
