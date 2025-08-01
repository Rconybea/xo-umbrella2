/** @file MachPipeline.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

//#include <cstdint>

#include "xo/refcnt/Refcounted.hpp"
#include "IrPipeline.hpp"
#include "LlvmContext.hpp"
#include "Jit.hpp"
#include "activation_record.hpp"

#include "xo/expression/Expression.hpp"
#include "xo/expression/ConstantInterface.hpp"
#include "xo/expression/PrimitiveInterface.hpp"
#include "xo/expression/Apply.hpp"
#include "xo/expression/Lambda.hpp"
#include "xo/expression/Variable.hpp"
#include "xo/expression/IfExpr.hpp"
#include "xo/expression/GlobalEnv.hpp"

/* stuff from kaleidoscope.cpp */
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include <llvm/ExecutionEngine/Orc/Core.h>


namespace xo {
    namespace jit {
        /** @class MachPipeline
         *  @brief just-in-time compiler for EGAD
         *
         *  TODO: make module name a parameter?
        **/
        class MachPipeline : public ref::Refcount {
        public:
            using Expression = xo::scm::Expression;
            using Lambda = xo::scm::Lambda;
            using GlobalEnv = xo::scm::GlobalEnv;
            using TypeDescr = xo::reflect::TypeDescr;
            using ExecutionSession = llvm::orc::ExecutionSession;
            using DataLayout = llvm::DataLayout;
            //using ConstantInterface = xo::scm::ConstantInterface;

        public:
            /* tracking KaleidoscopeJIT::Create() here.. */
            static llvm::Expected<std::unique_ptr<MachPipeline>> make_aux();
            static rp<MachPipeline> make();

            // ----- access -----

            llvm::Module * current_module() { return llvm_module_.get(); }
            bp<LlvmContext> llvm_cx() { return llvm_cx_; }
            llvm::IRBuilder<> * llvm_current_ir_builder() { return llvm_toplevel_ir_builder_.get(); }

            /** target triple = string describing target host for codegen **/
            const std::string & target_triple() const;
            /** execution session (run jit-generated machine code in this process) **/
            const ExecutionSession * xsession() const;
            /** data layout = rules for alignment/padding; specific to target host **/
            const DataLayout & data_layout() const;
            /** append function names defined in attached module to *p_v
             *
             *  (RC 15jun2024 - this part is working)
             **/
            std::vector<std::string> get_function_name_v();

            /** write state of execution session (all the associated dynamic libraries) **/
            void dump_execution_session();

            // ----- code generation -----

            /** establish llvm IR corresponding to a c++ type.
             *  Handles
             *  T := bool|char|short|int|long|float|double
             *        | T1(*)(T2..Tn)
             *        | struct{T1,..,Tn}
             *
             *  Not supported yet:
             *  - vector<T>
             *  - string
             *  - map<T1,T2>
             *  - unions
             *  - pointers (except function pointers)
             *
             *  Idempotent: multiple calls with the same @p td produce the same @c llvm::Type pointer.
             *  @c llvm::Type instances are *immortal* (llvm interns them into opaque global lookup tables)
             **/
            llvm::Type * codegen_type(TypeDescr td);
            llvm::Value * codegen_constant(bp<xo::scm::ConstantInterface> expr);
            llvm::Function * codegen_primitive(bp<xo::scm::PrimitiveInterface> expr);

            /** like @ref codegen_primitive , but create wrapper function that accepts (and discards)
             *  environment pointer as first argument.
             *
             *  Implementation consists of tail call to natural primitive, that skips the unused
             *  environment pointer
             **/
            llvm::Function * codegen_primitive_wrapper(bp<xo::scm::PrimitiveInterface> expr,
                                                       llvm::IRBuilder<> & ir_builder);

            /** Generate closure for invoking a primitive function.
             *  Primitives don't benefit from a closure, but we need a consistent ABI
             *  to support function-pointer-like behavior for a target function
             *  that may resolve to primitive-or-lambda at runtime
             **/
            llvm::Value * codegen_primitive_closure(bp<xo::scm::PrimitiveInterface> expr,
                                                    llvm::IRBuilder<> & ir_builder);

            llvm::Value * codegen_apply(bp<xo::scm::Apply> expr,
                                        llvm::Value * envptr,
                                        llvm::IRBuilder<> & ir_builder);
            /* NOTE: codegen_lambda() needs to be reentrant too.
             *       for example can have a lambda in apply position.
             */
            llvm::Function * codegen_lambda_decl(bp<xo::scm::Lambda> expr);
            llvm::Function * codegen_lambda_defn(bp<xo::scm::Lambda> expr, llvm::IRBuilder<> & ir_builder);
            /** Generate closure for invoking a lambda (user-defined function).
             *  See @ref MachPipeline::codegen_apply for invocation
             *  Same ABI as @ref MachPipeline::codegen_primitive_closure
             *
             *  @param envptr.  Environment from surrounding lexical scope.
             *                  This will be captured as envptr member by
             *                  the IR code for creating a closure.
             *                  @ref MachPipeline::codegen_toplevel and friends are responsible for
             *                  assembling and propagating this.
             **/
            llvm::Value * codegen_lambda_closure(bp<xo::scm::Lambda> lambda,
                                                 llvm::Value * envptr,
                                                 llvm::IRBuilder<> & ir_builder);
            llvm::Value * codegen_variable(bp<xo::scm::Variable> var,
                                           llvm::Value * envptr,
                                           llvm::IRBuilder<> & ir_builder);
           llvm::Value * codegen_ifexpr(bp<xo::scm::IfExpr> ifexpr,
                                         llvm::Value * envptr,
                                         llvm::IRBuilder<> & ir_builder);

            llvm::Value * codegen(bp<Expression> expr,
                                  llvm::Value * envptr,
                                  llvm::IRBuilder<> & ir_builder);

            llvm::Value * codegen_toplevel(bp<Expression> expr);

            // ----- jit online execution -----

            /** add IR code in current module to JIT,
             *  so that its available for execution
             **/
            void machgen_current_module();

            /** dump text description of module contents to console **/
            void dump_current_module();

            /** report mangled symbol for @p x **/
            std::string_view mangle(const std::string & x) const;

            /** lookup symbol in jit-associated output library **/
            llvm::Expected<llvm::orc::ExecutorAddr> lookup_symbol(const std::string & x);

            virtual void display(std::ostream & os) const;
            virtual std::string display_string() const;

        private:
            /** construct instance, adopting jit for compilation+execution **/
            explicit MachPipeline(std::unique_ptr<Jit> jit);

            /** iniitialize native builder (i.e. for platform we're running on) **/
            static void init_once();

            /** helper function.  find all lambda expressions in AST @p expr **/
            std::vector<bp<Lambda>> find_lambdas(bp<Expression> expr) const;

        public:
            /** codegen helper for a user-defined function.
             *  create stack slot on behalf of formal parameters.
             *  linked to (dynamic) callers for stack unwinding
             **/
            llvm::AllocaInst * create_entry_frame_alloca(llvm::Function * llvm_fn,
                                                         llvm::StructType * frame_llvm_type);

#ifdef OBSOLETE  // see activation_record::create_entry_block_alloca()
            /** codegen helper for a user-defined function (codegen_lambda()):
             *  create stack slot on behalf of some formal parameter to a function,
             *  so we can avoid SSA restriction on function body
             *
             *  @p var_type.  variable type
             **/
            llvm::AllocaInst * create_entry_block_alloca(llvm::Function * llvm_fn,
                                                         const std::string & var_name,
                                                         TypeDescr var_type);
#endif

        private:
            /** (re)create pipeline to turn expressions into llvm IR code **/
            void recreate_llvm_ir_pipeline();

        private:
            // ----- this part adapted from LLVM 19.0 KaleidoscopeJIT.hpp [wip] -----

            /** just-in-time compiler -- construct machine code that can
             *  be invoked from this running process
             **/
            std::unique_ptr<Jit> jit_;

            // ----- this part adapted from kaleidoscope.cpp -----

            /** everything below represents a pipeline
             *  that takes expressions, and turns them into llvm IR.
             *
             *  llvm IR can be added to running JIT by calling
             *    jit_->addModule()
             *  Note that this makes the module itself unavailable to us
             **/
            rp<IrPipeline> ir_pipeline_;

            /** owns + manages core "global" llvm data,
             *  including type- and constant- unique-ing tables.
             *
             *  Not threadsafe,  but ok to have multiple threads,
             *  each with its own LLVMContext
             **/
            rp<LlvmContext> llvm_cx_;

            /** builder for intermediate-representation objects **/
            std::unique_ptr<llvm::IRBuilder<>> llvm_toplevel_ir_builder_;

            /** a module (1:1 with library ?) being prepared by llvm.
             *  IR-level -- does not contain machine coode
             *
             *  - function names are unique within a module.
             **/
            std::unique_ptr<llvm::Module> llvm_module_;

            /** map global names to functions/variables **/
            rp<GlobalEnv> global_env_;

            /** map variable names (formal parameters) to
             *  corresponding llvm IR.
             *
             *  only supports one level atm (i.e. only top-level functions)
             *
             *  All values  live on the stack, so that we can evade single-assignment
             *  restrictions.
             *
             *  rhs identifies logical stack location of a variable
             **/
            std::stack<activation_record> env_stack_;  /* <-> kaleidoscope NamedValues */
        }; /*MachPipeline*/

        inline std::ostream &
        operator<<(std::ostream & os, const MachPipeline & x) {
            x.display(os);
            return os;
        }
    } /*namespace jit*/
} /*namespace xo*/


/** end MachPipeline.hpp **/
