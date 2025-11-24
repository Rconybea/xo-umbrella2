/** @file VirtualSchematikaMachine.cpp **/

#include "VirtualSchematikaMachine.hpp"
#include "VsmInstr.hpp"
#include "ExpressionBoxed.hpp"
#include "xo/expression/ConstantInterface.hpp"
#include "xo/expression/DefineExpr.hpp"
#include "xo/expression/Variable.hpp"
#include "xo/alloc/GC.hpp"

/** continue after completing a VSM instruction;
 *  achieve by jumping to continuation.
 **/
#define VSM_CONTINUE() this->pc_ = this->cont_; return;

/** report error and terminate VSM execution
 **/
#define VSM_ERROR(msg) report_error(msg); return;



namespace xo {
    using xo::gc::GC;

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

            /** assign variable after evaluating rhs of a define-expression
             *  - opcode is Opcode::defexpr_assign
             *  - top stack frame contains {lhs, cont}
             */
            static VsmInstr defexpr_assign_op;
        };

        VsmInstr
        VsmOps::halt_op{VsmInstr::Opcode::halt, "halt"};

        VsmInstr
        VsmOps::eval_op{VsmInstr::Opcode::eval, "eval"};

        VsmInstr
        VsmOps::defexpr_assign_op{VsmInstr::Opcode::defexpr_assign, "defexpr-assign"};

        // ----- VirtualSchematikaMachineFlyweight -----

        VirtualSchematikaMachineFlyweight::VirtualSchematikaMachineFlyweight(gc::IAlloc * mm,
                                                                             gp<Env> env,
                                                                             log_level ll) :
            object_mm_{mm},
            toplevel_env_{env},
            log_level_{ll}
        {}

        // ----- VirtualSchematikaMachine -----

        VirtualSchematikaMachine::VirtualSchematikaMachine(gc::IAlloc * mm,
                                                           gp<Env> env,
                                                           log_level ll) : flyweight_{mm, env, ll}
        {
            this->env_ = env;

            // gc roots
            gc::GC * gc = GC::from(mm);

            if (gc) {
                assert((gc->gc_in_progress() == false) && "cannot add roots while GC running");

                gc->add_gc_root_dwim(&env_);
                gc->add_gc_root_dwim(&value_);
            } else {
                // Want to support VSM with arena-allocator-only;
                // if only for unit testing.
            }

            // TODO: install builtin primitives here
        }

        VirtualSchematikaMachine::~VirtualSchematikaMachine()
        {
            gc::GC * gc = GC::from(flyweight_.object_mm_);

            if (gc) {
                assert((gc->gc_in_progress() == false) && "cannot remove roots while GC running");

                gc->remove_gc_root_dwim(&env_);
                gc->remove_gc_root_dwim(&value_);
            } else {
                // nothing to do in arena-only mode
            }
        }

        std::pair<gp<Object>,
                  SchematikaError>
        VirtualSchematikaMachine::toplevel_eval(bp<Expression> expr)
        {
            return this->eval(expr, this->env_);
        }

        std::pair<gp<Object>,
                  SchematikaError>
        VirtualSchematikaMachine::eval(bp<Expression> expr, gp<Env> env)
        {
            this->pc_    = &VsmOps::eval_op;
            this->expr_  = expr.promote();
            this->env_   = env;
            this->stack_ = nullptr;
            this->value_ = nullptr;
            this->error_ = SchematikaError();
            this->cont_  = &VsmOps::halt_op;

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
            scope log(XO_DEBUG(true));
            log && log("stack", stack_);

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
                    log && log("eval");

                    /* generally speaking: opcode will be 1:1 with extypes */

                    switch (expr_->extype()) {
                    case exprtype::constant:
                        this->eval_constant_op();
                        break;


                    case exprtype::define:
                        this->eval_define_op();
                        break;

                    case exprtype::invalid:
                    case exprtype::primitive:

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
                        this->error_ = SchematikaError(tostr("execute_vsm: not implemented",
                                                             xtag("extype", expr_->extype())));
                        this->cont_ = nullptr;
                        break;
                    }
                }
                break;

            case Opcode::defexpr_assign:
                this->do_defexpr_assign_op();
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
        VirtualSchematikaMachine::eval_constant_op()
        {
            using xo::scm::ConstantInterface;

            scope log(XO_DEBUG(true));

            bp<ConstantInterface> expr = ConstantInterface::from(expr_);

            assert(expr);

            this->value_ = flyweight_.object_converter_.tp_to_object(flyweight_.object_mm_,
                                                                     expr->value_tp(),
                                                                     false);
            if (this->value_.ptr()) {
                log && log("got object: ", xtag("value", value_));

                VSM_CONTINUE();
            } else {
                /* see ObjectConverter::ctor to add more builtin types */

                VSM_ERROR(tostr("constant_op: unable to convert native value to object",
                                xtag("id", expr->value_tp().td()->id()),
                                xtag("short_name", expr->value_tp().td()->short_name())));
            }
        }

        // placeholder: primitive_op

        void
        VirtualSchematikaMachine::eval_define_op()
        {
            using xo::scm::DefineExpr;

            scope log(XO_DEBUG(true));

            auto mm = flyweight_.object_mm_;

            bp<DefineExpr> expr = DefineExpr::from(expr_);

            assert(expr);
            assert(env_.get());

            // note: expecting nested define to have expanded iteself into
            //       applying a lambda

            // note: establish lhs_var first, to allow for recursion, for example:
            //        def fact(n: i64) { if (n == 0) then 1; else n * fact(n-1); }

            /** remembers promised variable type **/
            this->env_->establish_var(expr->lhs_variable());

            /** must promote rp<Expression> -> gp<ExpressionBoxed> **/
            gp<ExpressionBoxed> lhs_0 = ExpressionBoxed::make(mm, expr->lhs_variable());

            this->pc_ = &VsmOps::eval_op;
            this->expr_ = expr->rhs();

            /* when control arrives at .cont_, will have:
             *   .value_ -> result of evaluating expr->rhs()
             */

            this->stack_ = VsmStackFrame::push1(mm, this->stack_, lhs_0, cont_);
            this->cont_ = &VsmOps::defexpr_assign_op;
        }

        void
        VirtualSchematikaMachine::do_defexpr_assign_op()
        {
            /*
             * - value: contains result of evaluating rhs of define
             * - stack: top frame has 1 slot, holds variable to receive assignment
             */
            assert(value_.get());
            assert(stack_.get());

            gp<VsmStackFrame> sp0 = this->stack_;

            //this->value_ = this->value_; // preserve value from rhs of defexpr

            this->stack_ = sp0->parent();
            this->pc_ = this->cont_ = sp0->continuation();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end VirtualSchematikaMachine.cpp */
