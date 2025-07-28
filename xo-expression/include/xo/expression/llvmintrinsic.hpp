/** @file llvmintrinsic.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

//#include <cstdint>

namespace xo {
    namespace scm {
        /** @enum llvminstrinsic
         *  @brief enum to identify an LLVM instrinsic, e.g. @c IRBuilder::CreateFAdd
         *
         *  Associate an @c llvminstrinsic with an AST @c Primitive p.
         *  Later, in @c xo::jit::IrPipeline
         *  - when generating code for @c xo::scm::Apply
         *  - with *p* is in the function-call position
         *  can use the associated llvm instrinsic instead of generating a function call
         *  @c Primitive::value
         *
         *  @note llvm will still sometimes need to use
         *  @c Primitive::value (and generate a function call sequence),
         *  for example when handling an @c xo::scm::Apply instance
         *  where the function position is a computed function.
         *  @endnote
         *
         *  @note
         *  LLVM requires separate intrinsics for {ints, floats}.
         *  It does not need separate intrinsics for different sizes.
         *  For example IRBuilder::CreateAdd works for
         *  {8-bit, 16-bit, 32-bit, 64-bit, 128-bit} x {signed, unsigned} integers
         *  Integer division is an exception;  need to choose between i_sdiv and i_udiv
         *  @endnote
         *
         *  @note
         *  NSW stands for 'no signed wrap' -> poison value if overflow (costs more)
         *  NUW stands for 'no unsigned wrap' -> poison value if overflow (costs more)
         *  @endnote
         *
         *  See: xo-jit/src/jit/MachPipeline.cpp
         **/
        enum class llvmintrinsic {
            // see /nix/store/x5yz...llvm-18.1.5-dev/include/llvm/IR/IRBuilder.h

            /** sentinel value **/
            invalid = -1,

            /** -> IRBuilder::CreateNeg (negate 1 integer) **/
            i_neg,

            /** -> IRBuilder::CreateAdd (add 2 integers, overflow silently) **/
            i_add,

            /** -> IRBuilder::CreateSub (subtract 2 integers, overflow silently) **/
            i_sub,

            /** -> IRBuilder::CreateMul (multiply 2 integers, overflow silently) **/
            i_mul,

            /** -> IRBuilder::CreateSdiv (divide 2 signed integers) **/
            i_sdiv,

            /** -> IRBuilder::CreateUdiv (divide 2 unsigned integers) **/
            i_udiv,

            /** -> IRBuilder::CreateICmpEQ (test integers for equality) **/
            i_eq,

            /** -> IRBuilder::CreateICmpNE (test integers for inequality) **/
            i_ne,

            /** -> IRBuilder::CreateICmpSGT (test signed integers for greater) **/
            i_sgt,

            /** -> IRBuilder::CreateICmpSGE (test signed integers for greater-or-equal) **/
            i_sge,

            /** -> IRBuilder::CreateICmpSLT (test signed integers for lesser) **/
            i_slt,

            /** -> IRBuilder::CreateCmpSLE (test signed integers for lesser-or-equal) **/
            i_sle,

            // TODO: unsigned comparisons

            /** -> IRBuilder::CreateFAdd (add 2 floating-point numbers) **/
            fp_add,

            /** -> IRBuilder::CreateFSub (subtract 2 floating-pointer numbers) **/
            fp_sub,

            /** -> IRBuilder::CreateFMul (multiply 2 floating-point numbers) **/
            fp_mul,

            /** -> IRBuilder::CreateFDiv (divide 2 floating-point numbers) **/
            fp_div,

            // TODO: floating-point comparisons

            /**
             *  want to do whatever llvm IR @c llvm.sqrt.f64 and friends do.
             *  Not sure if that's an always-available function of something else
             **/
            fp_sqrt,

            /** WIP **/
            fp_pow,

            /** WIP **/
            fp_sin,

            /** WIP **/
            fp_cos,

            /** WIP **/
            fp_tan,

            /** not an intrinsic.  comes last, counts entries **/
            n_intrinsic
        };

        inline const char *
        llvmintrinsic2str(llvmintrinsic x)
        {
            switch(x) {
            case llvmintrinsic::invalid: return "?llvminstrinsic";
            case llvmintrinsic::i_neg: return "i_neg";
            case llvmintrinsic::i_add: return "i_add";
            case llvmintrinsic::i_sub: return "i_sub";
            case llvmintrinsic::i_mul: return "i_mul";
            case llvmintrinsic::i_sdiv: return "i_sdiv";
            case llvmintrinsic::i_udiv: return "i_udiv";

            case llvmintrinsic::i_eq: return "i_eq";
            case llvmintrinsic::i_ne: return "i_ne";
            case llvmintrinsic::i_sgt: return "i_sgt";
            case llvmintrinsic::i_sge: return "i_sge";
            case llvmintrinsic::i_slt: return "i_slt";
            case llvmintrinsic::i_sle: return "i_sle";

            case llvmintrinsic::fp_add: return "fp_add";
            case llvmintrinsic::fp_sub: return "fp_sub";
            case llvmintrinsic::fp_mul: return "fp_mul";
            case llvmintrinsic::fp_div: return "fp_div";
            case llvmintrinsic::fp_sqrt: return "fp_sqrt";
            case llvmintrinsic::fp_pow: return "fp_pow";
            case llvmintrinsic::fp_sin: return "fp_sin";
            case llvmintrinsic::fp_cos: return "fp_cos";
            case llvmintrinsic::fp_tan: return "fp_tan";
            default: break;
            }

            return "???llvmintrinsic???";
        } /*llvmintrinsic2str*/
    } /*namespace scm*/
} /*namespace xo*/

/** end llvmintrinsic.hpp **/
