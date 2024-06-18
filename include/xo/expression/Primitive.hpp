/** @file Primitive.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "PrimitiveInterface.hpp"
//#include <cstdint>

namespace xo {
    namespace ast {
        /** @class Primitive
         *  @brief syntax for a constant that refers to a known function.
         *
         *  Two cases here:
         *  1. primitive refers to a function that is supported directly in llvm
         *     (e.g. floating-point addition)
         *  2. primitive refers to a compiled (C/C++) function that we can invoke at runtime
         *
         *  In any case, a primitive serves as both declaration and definition
         *  (May be possible to relax this to declaration-only using null value_ as sentinel..?)
         *
         *  @tparam FunctionPointer   a function-pointer type, e.g. double(*)(double).
         *          Must be in this "canonical form".  std::function<double(double)>
         *          won't work here.
         **/
        template <typename FunctionPointer>
        class Primitive: public PrimitiveInterface {
        public:
            using Reflect = xo::reflect::Reflect;
            using TaggedPtr = xo::reflect::TaggedPtr;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            static ref::rp<Primitive> make(const std::string & name,
                                           FunctionPointer fnptr) {
                TypeDescr fn_type = Reflect::require<FunctionPointer>();

                return new Primitive(fn_type, name, fnptr);
            }

            FunctionPointer value() const { return value_; }

            // ----- PrimitiveInterface -----

            virtual std::string const & name() const override { return name_; }

            // ----- ConstantInterface -----

            virtual TypeDescr value_td() const override { return value_td_; }
            virtual TaggedPtr value_tp() const override {
                /* note: idk why,  but need to spell this out in two steps with gcc 13.2 */
                const void * erased_cptr = &value_;
                void * erased_ptr = const_cast<void*>(erased_cptr);

                return TaggedPtr(value_td_, erased_ptr);
            }

            // ----- Expression -----

            virtual void display(std::ostream & os) const override {
                os << "<Primitive"
                   << xtag("name", name_)
                   << xtag("type", this->value_td()->short_name())
                   << xtag("value", this->value())
                   << ">";
            }

        private:
            Primitive(TypeDescr fn_type,
                      const std::string & name,
                      FunctionPointer fnptr)
                : PrimitiveInterface(fn_type),
                  name_{name},
                  value_td_{Reflect::require_function<FunctionPointer>()},
                  value_{fnptr}
                {
                    if (!value_td_->is_function())
                        throw std::runtime_error("Primitive: expected function pointer");
                    if (!value_td_->fn_retval())
                        throw std::runtime_error("Primitive: expected non-null function return value");
                }


        private:
            // from Expression:
            //   exprtype extype_

            /** name of this primitive, e.g. '+', 'sqrt' **/
            std::string name_;
            /** type description for FunctionPointer **/
            TypeDescr value_td_;
            /** address of executable function **/
            FunctionPointer value_;
        }; /*Primitive*/

        /** adopt function @p x as a callable primitive function named @p name **/
        template <typename FunctionPointer>
        ref::rp<Primitive<FunctionPointer>>
        make_primitive(const std::string & name, FunctionPointer x) {
            return Primitive<FunctionPointer>::make(name, x);
        }
    } /*namespace ast*/
} /*namespace xo*/


/** end Primitive.hpp **/
