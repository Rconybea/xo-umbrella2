/** @file VirtualSchematikaMachine.cpp **/

#include "VirtualSchematikaMachine.hpp"

#include "VsmInstr.hpp"

/** continue after completing a VSM instruction;
 *  achieve by jumping to continuation.
 **/
//#define VSM_CONTINUE() this->pc_ = this->cont_; return;


namespace xo {
    namespace scm {
        struct VsmOps {
            /** halt virtual scheme machine.
             *  This will cause innermost run() to return to its caller
             **/
            static VsmInstr halt_op;

            /** evaluate an expression.
             *  - opcode is Opcode::eval
             *  - expression in register @ref expr_
             **/
            static VsmInstr eval_op;
        };

        VsmInstr
        VsmOps::halt_op{VsmInstr::Opcode::halt, "halt"};

        VsmInstr
        VsmOps::eval_op{VsmInstr::Opcode::eval, "eval"};

        VirtualSchematikaMachine::VirtualSchematikaMachine()
        {}

        void
        VirtualSchematikaMachine::run()
        {
            for (const VsmInstr * pc = pc_; pc; pc = pc_)
                this->execute_vsm(pc);
        }

        void
        VirtualSchematikaMachine::execute_vsm(const VsmInstr * instr)
        {
            using Opcode = VsmInstr::Opcode;

            switch (instr->opcode()) {

            case Opcode::halt:
                {
                    this->pc_ = nullptr;
                    this->cont_ = nullptr;
                    break;
                }

            case Opcode::eval:
                {
                    /* generally speaking: opcode will be 1:1 with extypes */

                    switch (expr_->extype()) {
                    case exprtype::invalid:
                    case exprtype::constant:
                    case exprtype::primitive:
                    case exprtype::define:
                    case exprtype::assign:
                    case exprtype::apply:
                    case exprtype::lambda:
                    case exprtype::variable:
                    case exprtype::ifexpr:
                    case exprtype::sequence:
                    case exprtype::convert:
                    case exprtype::n_expr:
                        this->pc_ = nullptr;
                        this->value_ = nullptr;
                        this->error_ = SchematikaError(tostr("execute_vsm: not implmented",
                                                             xtag("extype", expr_->extype())));
                        this->cont_ = nullptr;
                        break;
                    }
                }
                break;

            case Opcode::N_Opcode:
                assert(false);
                break;
            }
        }

        std::pair<gp<Object>,
                  SchematikaError>
        VirtualSchematikaMachine::eval(bp<Expression> expr)
        {
            this->pc_   = &VsmOps::eval_op;
            this->expr_ = expr.promote();
            this->cont_ = &VsmOps::halt_op;

            this->run();

            return std::make_pair(this->value_, this->error_);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end VirtualSchematikaMachine.cpp */
