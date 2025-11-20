/** @file VirtualSchematikaMachine.hpp **/

#pragma once

#include "VsmInstr.hpp"
#include "SchematikaError.hpp"
#include "xo/expression/Expression.hpp"
#include "xo/alloc/Object.hpp"

namespace xo {
    namespace scm {
        /** @brief state that may be shared across VirtualSchematikaMachine instances **/
        struct VirtualSchematikaMachineFlyweight {
            /** convert TaggedPtr->Object **/
            ObjectConverter object_converter_;
        };

        /** @class VirtualSchematikaMachine
         *  @brief Virtual machine implementing a Schematika interpreter
         *
         **/
        class VirtualSchematikaMachine {
        public:
            VirtualSchematikaMachine();

            /** evaluate expression @p expr.
             *  borrows calling thread until completion
             *  return [value, error]. error ignored unless value is nullptr.
             *  conversely when value is nullptr, error gives details of original
             *  error.
             **/
            std::pair<gp<Object>, SchematikaError> eval(bp<Expression> expr);

        private:
            /** borrow calling thread to run schematika machine
             *  indefinitely, or until null continuation
             **/
            void run();

            /** execute vsm instruction @p pc
             *  Note: may possibly be able to replace with just opcode
             **/
            void execute_one(const VsmInstr * pc);

            /** implementation class; contains instruction implementations **/
            friend class VsmOps;

        private:
            /** program counter.
             *  (Perhaps replace with VsmInstr::Opcode ?)
             **/
            const VsmInstr * pc_ = nullptr;

            /** expression **/
            rp<Expression> expr_;

            /** non-error result value from eval() / apply() **/
            gp<Object> value_;

            /** error result value from eval() / apply() **/
            SchematikaError error_;

            /** continuation
             *  (Perhaps replace with VsmInstr::Opcode ?)
             **/
            const VsmInstr * cont_ = nullptr;

            /** possibly-shared data **/
            VirtualSchemtikaMachineFlyweight flyweight_;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end VirtualSchematikaMachine.hpp */
