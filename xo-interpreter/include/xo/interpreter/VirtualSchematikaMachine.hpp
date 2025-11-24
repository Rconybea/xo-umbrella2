/** @file VirtualSchematikaMachine.hpp **/

#pragma once

#include "VsmInstr.hpp"
#include "SchematikaError.hpp"
#include "Env.hpp"
#include "xo/expression/Expression.hpp"
#include "xo/object/ObjectConverter.hpp"
#include "xo/alloc/Object.hpp"

namespace xo {
    namespace scm {
        /** @brief state that may be shared across VirtualSchematikaMachine instances **/
        struct VirtualSchematikaMachineFlyweight {
            explicit VirtualSchematikaMachineFlyweight(gc::IAlloc * mm,
                                                       gp<Env> env);

            /** memory allocator for interpreter operation. **/
            gc::IAlloc * object_mm_ = nullptr;
            /** global environment **/
            gp<Env> toplevel_env_;
            /** convert TaggedPtr->Object **/
            xo::obj::ObjectConverter object_converter_;
        };

        /** @class VirtualSchematikaMachine
         *  @brief Virtual machine implementing a Schematika interpreter
         *
         **/
        class VirtualSchematikaMachine {
        public:
            using IAlloc = xo::gc::IAlloc;

        public:
            VirtualSchematikaMachine(IAlloc * mm, gp<Env> toplevel_env);
            ~VirtualSchematikaMachine();

            /** evaluate expression @p expr.
             *  borrows calling thread until completion
             *  return [value, error]. error ignored unless value is nullptr.
             *  conversely when value is nullptr, error gives details of original
             *  error.
             *
             *  Evaluate schematika expression @p expr in environment @p env
             **/
            std::pair<gp<Object>, SchematikaError> eval(bp<Expression> expr, gp<Env> env);

            /** evaluate expression @p expr in toplevel environment **/
            std::pair<gp<Object>, SchematikaError> toplevel_eval(bp<Expression> expr);

        private:
            /** Not moveable or copyable.
             *  One constraint is member variables added to flyweight_.object_mm_
             *  as GC roots, with no provision for unwinding later.
             **/
            VirtualSchematikaMachine(const VirtualSchematikaMachine &) = delete;
            VirtualSchematikaMachine(VirtualSchematikaMachine &&) = delete;

            /** borrow calling thread to run schematika machine
             *  indefinitely, or until null continuation
             **/
            void run();

            /** execute vsm instruction in program counter.
             *  Note: may possibly be able to replace with just opcode
             **/
            void execute_one();

            /** interpret literal constant expression **/
            void eval_constant_op();

            /** goto error state with message @p err **/
            void report_error(const std::string & err);

            /** implementation class; contains instruction implementations **/
            friend struct VsmOps;

        private:
            /** program counter.
             *  (Perhaps replace with VsmInstr::Opcode ?)
             **/
            const VsmInstr * pc_ = nullptr;

            /** expression **/
            rp<Expression> expr_;
            /** holds bindings for all schematika variables **/
            gp<Env> env_;

            /** non-error result value from eval() / apply() **/
            gp<Object> value_;

            /** error result value from eval() / apply() **/
            SchematikaError error_;

            /** continuation
             *  (Perhaps replace with VsmInstr::Opcode ?)
             **/
            const VsmInstr * cont_ = nullptr;

            /** possibly-shared data **/
            VirtualSchematikaMachineFlyweight flyweight_;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end VirtualSchematikaMachine.hpp */
