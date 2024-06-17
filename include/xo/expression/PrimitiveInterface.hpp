/** @file PrimitiveInterface.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "ConstantInterface.hpp"

//#include <cstdint>

#include <type_traits>
namespace xo {
    namespace ast {
        class PrimitiveInterface : public ConstantInterface {
        public:
            PrimitiveInterface() : ConstantInterface(exprtype::primitive) {}

            /** downcast from Expression **/
            static ref::brw<PrimitiveInterface> from(ref::brw<Expression> x) {
                return ref::brw<PrimitiveInterface>::from(x);
            }

            virtual const std::string & name() const = 0;
            int n_arg() const { return this->value_td()->n_fn_arg(); }
            TypeDescr fn_retval() const { return this->value_td()->fn_retval(); }
            TypeDescr fn_arg(uint32_t i) const { return this->value_td()->fn_arg(i); }

            //virtual TypeDescr value_td() const override = 0;
            //virtual TaggedPtr value_tp() const override = 0;

        private:
        }; /*PrimitiveInterface*/
    } /*namespace ast*/
} /*namespace xo*/


/** end PrimitiveInterface.hpp **/
