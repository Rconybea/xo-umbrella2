/** @file PrimitiveInterface.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "FunctionInterface.hpp"

//#include <cstdint>

#include <type_traits>
namespace xo {
    namespace ast {
        class PrimitiveInterface : public FunctionInterface {
        public:
            PrimitiveInterface(TypeDescr fn_type)
                : FunctionInterface(exprtype::primitive, fn_type) {}

            /** downcast from Expression **/
            static ref::brw<PrimitiveInterface> from(ref::brw<Expression> x) {
                return ref::brw<PrimitiveInterface>::from(x);
            }

            // virtual const std::string & name() const;
            // virtual int n_arg() const;
            // virtual TypeDescr fn_retval() const;
            // virtual TypeDescr fn_arg(uint32_t i) const;

        private:
        }; /*PrimitiveInterface*/
    } /*namespace ast*/
} /*namespace xo*/


/** end PrimitiveInterface.hpp **/
