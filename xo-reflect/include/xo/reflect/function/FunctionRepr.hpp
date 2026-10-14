/** @file FunctionRepr.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

namespace xo {
    namespace reflect {
        /** @brief how a callable (Metatype::mt_function) is represented.
         *
         *  One metatype means "callable"; this tells the representations
         *  apart, for consumers that care -- code that calls through a raw
         *  function pointer cannot call a type-erased std::function.
         *  Part of a function type's interning key (FunctionTdxInfo): the
         *  same signature in two representations is two C++ types, two
         *  type descriptions.  See .xo-backlog/xo-reflect/issues/09
         **/
        enum class FunctionRepr {
            /** a function pointer:  R (*)(A..) **/
            pointer,
            /** a std::function<R (A..)> **/
            std_function,
        };
    } /*namespace reflect*/
} /*namespace xo*/

/** end FunctionRepr.hpp **/
