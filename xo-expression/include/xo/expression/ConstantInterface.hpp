/** @file ConstantInterface.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include <type_traits>

namespace xo {
    namespace scm {
        /** @class ConstantInterface
         *  @brief syntax for a literal constant.
         **/
        class ConstantInterface : public Expression {
        public:
            using TaggedPtr = xo::reflect::TaggedPtr;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            /** @p extype sets expression-type;  could be constant|primitive **/
            ConstantInterface(exprtype extype, TypeDescr valuetype) : Expression{extype, valuetype} {}

            /** downcast from Expression **/
            static bp<ConstantInterface> from(bp<Expression> x) {
                return bp<ConstantInterface>::from(x);
            }

            /** type description for representation of literal value **/
            virtual TypeDescr value_td() const = 0;
            /** reflection-tagged pointer to literal value of this constant **/
            virtual TaggedPtr value_tp() const = 0;

            // ----- Expression -----

            virtual std::set<std::string> get_free_variables() const override {
                return std::set<std::string>();
            }

            virtual void attach_envs(bp<SymbolTable> /*p*/) override {}

        }; /*ConstantInterface*/

    } /*namespace scm*/
} /*namespace xo*/

/** end ConstantInterface.hpp **/
