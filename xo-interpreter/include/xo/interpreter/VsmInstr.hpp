/** @file VsmInstr.hpp **/

#pragma once

#include <string_view>

namespace xo {
    namespace scm {
        class VirtualSchematikaMachine; // see VirtualSchematikaMachine.hpp

        /** @class VmInstr
         *  @brief Represent a particular vritual schematika machine instruction
         *
         *  A vsm instruction acts on a virtual schematika machine instance.
         **/
        class VsmInstr
        {
        public:
            enum class Opcode {
                /** Halt virtual schematika machine **/
                halt,

                /** Evaluate a schematika expression.
                 *  See VirtualSchematikaMachine::eval()
                 **/
                eval,

                /** assign to variable + continue
                 *
                 *  stack: frame with:
                 *    [0] lhs : variable to assign
                 **/
                complete_assign,

                /** execute ifexpr branch, given
                 *  result of test expression has been established
                 **/
                complete_ifexpr,

                /** execute remainder of expression sequence
                 **/
                complete_sequence,

                /** choose branch of if-expression + continue
                 *
                 *  stack: frame with
                 *    [0] ifexpr : original if-expression
                 **/
                N_Opcode
            };

            //using ActionFn = void (*)(VirtualSchematikaMachine * vm);

        public:
            VsmInstr(Opcode opcode, std::string_view name);

            Opcode opcode() const { return opcode_; }

        private:
            /** unique opcode for this instruction **/
            Opcode opcode_;
            /** **/
            std::string_view name_;
            //ActionFn action_;
        };
    }
}

/* end VsmInstr.hpp */
