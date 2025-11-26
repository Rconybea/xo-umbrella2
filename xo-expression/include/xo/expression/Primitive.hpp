/** @file Primitive.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "PrimitiveExprInterface.hpp"
#include "pretty_expression.hpp"
#include "llvmintrinsic.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/indentlog/print/quoted.hpp"

extern "C" {
    /* these symbols needed to link primitives */

    /* see Primitive_f64::make() */
    double add2_f64(double x, double y);
};

namespace xo {
    namespace scm {
        /** @class Primitive
         *  @brief syntax for a constant that refers to a known function.
         *
         *  Two cases here:
         *  1. (always) primitive refers to a compiled (C/C++) function that we can invoke at runtime
         *  2. (sometimes) primitive also refers to a function that is supported directly in llvm
         *     (e.g. floating-point addition).  In that case @ref intrinsic_
         *     identifies that direct support, provided it knows at codegen time which primitive
         *     is being invoked
         *
         *  In any case, a primitive serves as both declaration and definition
         *  (May be possible to relax this to declaration-only using null value_ as sentinel..?)
         *
         *  @tparam FunctionPointer   a function-pointer type, e.g. double(*)(double).
         *          Must be in this "canonical form".  std::function<double(double)>
         *          won't work here.
         **/
        template <typename FunctionPointer>
        class Primitive: public PrimitiveExprInterface {
        public:
            using Reflect = xo::reflect::Reflect;
            using TaggedPtr = xo::reflect::TaggedPtr;
            using TypeDescr = xo::reflect::TypeDescr;
            using fptr_type = FunctionPointer;

        public:
            static rp<Primitive> make(const std::string & name,
                                      FunctionPointer fnptr,
                                      bool explicit_symbol_def,
                                      llvmintrinsic intrinsic) {
                TypeDescr fn_type = Reflect::require<FunctionPointer>();

                return new Primitive(fn_type, name, fnptr, explicit_symbol_def, intrinsic);
            }

            /** see classes below for intrinsics **/

            FunctionPointer value() const { return value_; }

            TypeDescr value_td() const { return value_td_; }
            TaggedPtr value_tp() const {
                /* note: idk why,  but need to spell this out in two steps with gcc 13.2 */
                const void * erased_cptr = &value_;
                void * erased_ptr = const_cast<void*>(erased_cptr);

                return TaggedPtr(value_td_, erased_ptr);
            }

            // ----- PrimitiveInterface -----

            virtual llvmintrinsic intrinsic() const override { return intrinsic_; }
            virtual bool explicit_symbol_def() const override { return explicit_symbol_def_; }
            virtual void_function_type function_address() const override { return reinterpret_cast<void_function_type>(value_); }

            // ----- FunctionInterface -----

            virtual std::string const & name() const override { return name_; }
            virtual int n_arg() const override { return this->value_td()->n_fn_arg(); }
            virtual TypeDescr fn_retval() const override { return this->value_td()->fn_retval(); }
            virtual TypeDescr fn_arg(uint32_t i) const override { return this->value_td()->fn_arg(i); }

            // ----- Expression -----

            virtual void display(std::ostream & os) const override {
                os << "<Primitive"
                   << xtag("name", name_)
                   << xtag("type", this->value_td()->short_name())
                   << xtag("value", this->value())
                   << ">";
            }

            virtual std::uint32_t pretty_print(const ppindentinfo & ppii) const override {
                /* 1. rtag instead of refrtag:
                 *    print::quot() is a temporary rvalue; lifetime ends before control enters pretty_struct()
                 *
                 * 2. value cast to void*:
                 *    we don't have pretty printer for native function pointers anyway
                 *    + simplifies ppdetail_atomic
                 */
                return ppii.pps()->pretty_struct(ppii, "Primitive",
                                                 refrtag("name", name_),
                                                 rtag("type", print::quot(this->valuetype()->short_name())),
                                                 refrtag("value", (void*)(this->value())));
            }

        private:
            Primitive(TypeDescr fn_type,
                      const std::string & name,
                      FunctionPointer fnptr,
                      bool explicit_symbol_def,
                      llvmintrinsic intrinsic)
                : PrimitiveExprInterface(fn_type),
                  name_{name},
                  value_td_{Reflect::require_function<FunctionPointer>()},
                  value_{fnptr},
                  explicit_symbol_def_{explicit_symbol_def},
                  intrinsic_{intrinsic}
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
            /** for LLVM: if true,  use Jit.intern_symbol() to provide explicit binding.
             *
             *  Not obvious what distinguishes functions like ::sin(), ::sqrt()
             *  (which work without this) from symbols like ::mul_i32(), which require it.
             **/
            bool explicit_symbol_def_ = false;
            /** invalid:    generate call (IRBuilder::CreateCall)
             *  all others: generate direct use of LLVM intrinsic
             **/
            llvmintrinsic intrinsic_;
        }; /*Primitive*/

        /** adopt function @p x as a callable primitive function named @p name **/
        template <typename FunctionPointer>
        rp<Primitive<FunctionPointer>>
        make_primitive(const std::string & name,
                       FunctionPointer x,
                       bool explicit_symbol_def,
                       llvmintrinsic intrinsic)
        {
            return Primitive<FunctionPointer>::make(name, x, explicit_symbol_def, intrinsic);
        }

        // NOTE: see xo-reader/src/reader/progress_xs.cpp
        //       binding operators to primitive applications.

        /** builtin primitives :: i64 x i64 -> bool **/
        class Primitive_cmp_i64 : public Primitive<bool (*)(std::int64_t, std::int64_t)> {
        public:
            using PrimitiveType = Primitive<bool (*)(std::int64_t, std::int64_t)>;

        public:
            /** eq2_i64: compare two 64-bit integers for equality **/
            static rp<PrimitiveType> make_cmp_eq2_i64();
            /** ne2_i64: compare two 64-bit integers for inequality **/
            static rp<PrimitiveType> make_cmp_ne2_i64();
            /** lt2_i64: compare two 64-bit integers for lessthan **/
            static rp<PrimitiveType> make_cmp_lt2_i64();
            /** lt2_i64: compare two 64-bit integers for lessthanorequal **/
            static rp<PrimitiveType> make_cmp_le2_i64();
            /** gt2_i64: compare two 64-bit integers for greaterthan **/
            static rp<PrimitiveType> make_cmp_gt2_i64();
            /** ge2_i64: compare two 64-bit integers for greaterthan **/
            static rp<PrimitiveType> make_cmp_ge2_i64();
        };

        /** builtin primitives :: i64 x i64 -> i64 **/
        class Primitive_i64 : public Primitive<std::int64_t (*)(std::int64_t, std::int64_t)> {
        public:
            using PrimitiveType = Primitive<std::int64_t (*)(std::int64_t, std::int64_t)>;

        public:
            /** add2_i64: add two 64-bit integers **/
            static rp<PrimitiveType> make_add2_i64();
            /** sub2_i64: subtract two 64-bit integers **/
            static rp<PrimitiveType> make_sub2_i64();
            /** mul2_i64: multiply two 64-bit integers **/
            static rp<PrimitiveType> make_mul2_i64();
            /** div2_i64: divide two 64-bit integers **/
            static rp<PrimitiveType> make_div2_i64();
        };

        /** builtin primitives :: f64 x f64 -> f64 **/
        class Primitive_f64 : public Primitive<double (*)(double, double)> {
        public:
            using PrimitiveType = Primitive<double (*)(double, double)>;

        public:
            /** add2_f64: add two 64-bit floating-point numbers **/
            static rp<PrimitiveType> make_add2_f64();
            /** sub2_f64: subtract two 64-bit floating-point numbers **/
            static rp<PrimitiveType> make_sub2_f64();
            /** mul2_f64: multiply two 64-bit floating-point numbers **/
            static rp<PrimitiveType> make_mul2_f64();
            /** div2_f64: divide two 64-bit floating-point numbers **/
            static rp<PrimitiveType> make_div2_f64();
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end Primitive.hpp **/
