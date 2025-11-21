/** @file VirtualSchematikaMachine.cpp **/

#include "VirtualSchematikaMachine.hpp"
#include "VsmInstr.hpp"
#include "xo/expression/ConstantInterface.hpp"

/** continue after completing a VSM instruction;
 *  achieve by jumping to continuation.
 **/
#define VSM_CONTINUE() this->pc_ = this->cont_; return;

/** report error and terminate VSM execution
 **/
#define VSM_ERROR(msg) report_error(msg); return;

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

        // ----- VirtualSchematikaMachineFlyweight -----

        VirtualSchematikaMachineFlyweight::VirtualSchematikaMachineFlyweight(gc::IAlloc * mm) :
            object_mm_{mm}
        {}

        // ----- VirtualSchematikaMachine -----

        VirtualSchematikaMachine::VirtualSchematikaMachine(gc::IAlloc * mm) : flyweight_{mm}
        {}

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

        void
        VirtualSchematikaMachine::run()
        {
            while(pc_)
                this->execute_one();
        }

        void
        VirtualSchematikaMachine::execute_one()
        {
            using Opcode = VsmInstr::Opcode;

            switch (pc_->opcode()) {

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
                    case exprtype::constant:
                        this->constant_op();
                        break;

                    case exprtype::invalid:
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

        void
        VirtualSchematikaMachine::report_error(const std::string & err)
        {
            /* error short-circuits vsm operation */

            this->pc_ = nullptr;
            this->value_ = nullptr;
            this->error_ = SchematikaError(err);
            this->cont_ = nullptr;
        }

        void
        VirtualSchematikaMachine::constant_op()
        {
            scope log(XO_DEBUG(true));

            using xo::scm::ConstantInterface;

            bp<ConstantInterface> expr = ConstantInterface::from(expr_);

            assert(expr);

            this->value_ = flyweight_.object_converter_.tp_to_object(flyweight_.object_mm_,
                                                                     expr->value_tp(),
                                                                     false);
            if (this->value_.ptr()) {
                log && log("got object: ", xtag("value", value_));

                VSM_CONTINUE();
            } else {
                VSM_ERROR(tostr("constant_op: unable to convert native value to object",
                                xtag("id", expr->value_tp().td()->id()),
                                xtag("short_name", expr->value_tp().td()->short_name())));
            }
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end VirtualSchematikaMachine.cpp */
