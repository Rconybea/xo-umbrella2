/** @file VirtualSchematikaMachine.cpp **/

#include "VirtualSchematikaMachine.hpp"
#include "VsmInstr.hpp"
#include "BuiltinPrimitives.hpp"
#include "ExpressionBoxed.hpp"
#include "xo/expression/Constant.hpp"
#include "xo/expression/PrimitiveExprInterface.hpp"
#include "xo/expression/DefineExpr.hpp"
#include "xo/expression/AssignExpr.hpp"
#include "xo/expression/Variable.hpp"
#include "xo/expression/IfExpr.hpp"
#include "xo/expression/Sequence.hpp"
#include "xo/expression/Apply.hpp"
#include "xo/object/Procedure.hpp"
#include "xo/object/Primitive.hpp"
#include "xo/object/Integer.hpp"
#include "xo/object/Boolean.hpp"
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
    using xo::obj::Procedure;
    using xo::obj::Integer;
    using xo::obj::Boolean;

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

            /** assign variable after evaluating rhs of a define-expression or assign-expression
             *  - opcode is Opcode::complete_assign
             *  - top stack frame contains {lhs, cont}
             **/
            static VsmInstr complete_assign_op;

            /** choose branch of if-expr after evaluating test condition.
             *  - opcode is Opcode::complete_ifexpr
             *  - top stack frame contains {ifexpr, cont}
             **/
            static VsmInstr complete_ifexpr_op;

            /** proceed to next element of sequence-expr.
             *  - opcode is Opcode::complete_sequence
             *  - top stack frame contains {seq, next, cont}
             */
            static VsmInstr complete_sequence_op;

            /** proceed to next argument in apply-expr
             *  - opcode is Opcode::eval_collect_args
             *  - top stack frame contains {apply, targetarg, cont}
             */
            static VsmInstr complete_evalargs_op;

            /** call a procedure, where evaluated arguments (including target function)
             *  are in top stack frame.
             *  - opcode is Opcode::apply
             *  - top stack frame contains evaluated arguments.
             **/
            static VsmInstr apply_op;
        };

        VsmInstr
        VsmOps::halt_op{VsmInstr::Opcode::halt, "halt"};

        VsmInstr
        VsmOps::eval_op{VsmInstr::Opcode::eval, "eval"};

        VsmInstr
        VsmOps::complete_assign_op{VsmInstr::Opcode::complete_assign, "complete-assign"};

        VsmInstr
        VsmOps::complete_ifexpr_op{VsmInstr::Opcode::complete_ifexpr, "complete-ifexpr"};

        VsmInstr
        VsmOps::complete_sequence_op{VsmInstr::Opcode::complete_sequence, "complete-sequence"};

        VsmInstr
        VsmOps::complete_evalargs_op{VsmInstr::Opcode::complete_evalargs, "complete-evalargs"};

        VsmInstr
        VsmOps::apply_op{VsmInstr::Opcode::apply, "apply"};

        // ----- VirtualSchematikaMachineFlyweight -----

        VirtualSchematikaMachineFlyweight::VirtualSchematikaMachineFlyweight(gc::IAlloc * mm,
                                                                             gp<GlobalEnv> env,
                                                                             log_level ll) :
            object_mm_{mm},
            toplevel_env_{env},
            log_level_{ll}
        {
        }

        // ----- VirtualSchematikaMachine -----

        VirtualSchematikaMachine::VirtualSchematikaMachine(gc::IAlloc * mm,
                                                           gp<GlobalEnv> env,
                                                           log_level ll) : flyweight_{mm, env, ll}
        {
            scope log(XO_DEBUG(true), xtag("env", env), xtag("symtab", env->symtab()));

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
        VirtualSchematikaMachine::eval(bp<Expression> expr, gp<GlobalEnv> env)
        {
            scope log(XO_DEBUG(true), xtag("env", env), xtag("symtab", env->symtab()));

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
            log && log(xtag("pc", pc_), xtag("cont", cont_));
            log && log(xtag("stack", stack_));

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
                    log && log("Opcode::eval");

                    /* generally speaking: opcode will be 1:1 with extypes */

                    switch (expr_->extype()) {
                    case exprtype::constant:
                        log && log("eval -> constant");
                        this->eval_constant_op();
                        break;

                    case exprtype::primitive:
                        log && log("eval -> primitive");
                        this->eval_primitive_op();
                        break;

                    case exprtype::define:
                        log && log("eval -> define");
                        this->eval_define_op();
                        break;

                    case exprtype::assign:
                        log && log("eval -> assign");
                        this->eval_assign_op();
                        break;

                    case exprtype::variable:
                        log && log("eval -> variable");
                        this->eval_variable_op();
                        break;

                    case exprtype::ifexpr:
                        log && log("eval -> ifexpr");
                        this->eval_ifexpr_op();
                        break;

                    case exprtype::sequence:
                        log && log("eval -> sequence");
                        this->eval_sequence_op();
                        break;

                    case exprtype::apply:
                        log && log("eval -> apply");
                        this->eval_apply_op();
                        break;

                    case exprtype::invalid:

                    case exprtype::lambda:
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

            case Opcode::complete_assign:
                this->do_complete_assign_op();
                break;

            case Opcode::complete_ifexpr:
                this->do_complete_ifexpr_op();
                break;

            case Opcode::complete_sequence:
                this->do_complete_sequence_op();
                break;

            case Opcode::complete_evalargs:
                this->do_complete_evalargs_op();
                break;

            case Opcode::apply:
                this->apply_op();
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
                /* see ObjectConverter::ctor to add more builtin types
                 *
                 * generally conversion for a type Foo will appear in Foo.hpp
                 * see
                 *   xo/object/Boolean.hpp
                 *   xo/object/Integer.hpp
                 *   xo/object/Float.hpp
                 *   xo/object/String.hpp
                 */

                VSM_ERROR(tostr("constant_op: unable to convert native value to object",
                                xtag("id", expr->value_tp().td()->id()),
                                xtag("short_name", expr->value_tp().td()->short_name())));
            }
        }

        void
        VirtualSchematikaMachine::eval_primitive_op()
        {
            using xo::obj::Primitive;
            using xo::reflect::TaggedPtr;

            scope log(XO_DEBUG(true));

            bp<PrimitiveExprInterface> expr = PrimitiveExprInterface::from(expr_);

            const gp<Object> * slot = env_->lookup_slot(expr->name());

            if (slot) {
                this->value_ = *slot;
                this->pc_ = cont_;
            } else {
                std::string err = tostr("no binding for primitive", xtag("name", expr->name()));

                this->value_ = nullptr;
                this->error_ = SchematikaError(err);

                /* note: poor man's exception */
                this->pc_ = nullptr;
                this->cont_ = nullptr;
            }
        }

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

            /* .stack_:
             *   frame
             *     [0] = lhs_0 (boxed lhs Variable)
             *   ..
             */

            this->cont_ = &VsmOps::complete_assign_op;
        }

        void
        VirtualSchematikaMachine::eval_assign_op()
        {
            using xo::scm::AssignExpr;

            scope log(XO_DEBUG(true));

            auto mm = flyweight_.object_mm_;

            bp<AssignExpr> assign = AssignExpr::from(expr_);

            assert(assign.get());
            assert(env_.get());

            assert(assign->lhs().get());
            assert(assign->rhs().get());

            /* verify slot exists, before we evaluate rhs */
            gp<Object> * slot = env_->lookup_slot(assign->lhs()->name());

            if (slot) {
                /** must promote rp<Expression> -> gp<ExpressionBoxed> **/
                gp<ExpressionBoxed> lhs = ExpressionBoxed::make(mm, assign->lhs());

                this->pc_ = &VsmOps::eval_op;
                this->expr_ = assign->rhs();

                /* when control arrives at .cont_, will have:
                 *   .value_ -> result of evaluating assign->rhs()
                 */
                this->stack_ = VsmStackFrame::push1(mm, this->stack_, lhs, cont_);

                /* .stack_:
                 *   frame
                 *     [0] = lhs (boxed lhs Variable)
                 *   ..
                 */

                this->cont_ = &VsmOps::complete_assign_op;
            } else {
                std::string err = tostr("no binding for lhs of assignment", xtag("name", assign->lhs()->name()));

                this->value_ = nullptr;
                this->error_ = SchematikaError(err);

                /* note: poor man's exception */
                this->pc_ = nullptr;
                this->cont_ = nullptr;
            }
        }

        void
        VirtualSchematikaMachine::do_complete_assign_op()
        {
            scope log(XO_DEBUG(true));

            /*
             * - value: contains result of evaluating rhs of define
             * - stack: top frame has 1 slot, holds variable to receive assignment
             */
            assert(value_.get());
            assert(stack_.get());
            assert(env_.get());

            gp<VsmStackFrame> sp0 = this->stack_;

            bp<Variable> var = Variable::from(ExpressionBoxed::from((*sp0)[0])->contents());

            assert(var.get());

            gp<Object> * slot = this->env_->establish_var(var);
            assert(slot);

            *slot = this->value_;

            //this->value_ = this->value_; // preserve value from rhs of defexpr

            this->stack_ = sp0->parent();
            this->pc_ = this->cont_ = sp0->continuation();
        }

        void
        VirtualSchematikaMachine::eval_variable_op()
        {
            using xo::scm::Variable;

            scope log(XO_DEBUG(true));

            bp<Variable> var = Variable::from(expr_);

            assert(var.get());
            assert(env_.get());

            const gp<Object> * slot = env_->lookup_slot(var->name());

            if (slot) {
                this->value_ = *slot;
                this->pc_ = cont_;
            } else {
                /* Unknown variable error will often be recognized in expression parser,
                 * in such cases this path won't be used.
                 *
                 * In interactive environment will need some kind of support for modifying
                 * code (e.g. replacing top-level functions/variables), and in particular,
                 * replacements may have different type signature.
                 * It's possible that allowing for such replacements winds up giving up
                 * typesafety guarantees. In that case this path may get activated after
                 * all.
                 */

                std::string err = tostr("no binding for variable", xtag("name", var->name()));

                this->value_ = nullptr;
                this->error_ = SchematikaError(err);

                /* note: poor man's exception */
                this->pc_ = nullptr;
                this->cont_ = nullptr;
            }
        }

        void
        VirtualSchematikaMachine::eval_ifexpr_op()
        {
            using xo::scm::IfExpr;

            scope log(XO_DEBUG(true));

            gc::IAlloc * mm = flyweight_.object_mm_;

            /** must promote bp<IfExpr> -> gp<ExpressionBoxed> **/
            gp<ExpressionBoxed> ifexpr_boxed = ExpressionBoxed::make(mm, expr_);
            bp<IfExpr> ifexpr = IfExpr::from(expr_);

            assert(ifexpr.get());
            assert(env_.get());

            this->pc_ = &VsmOps::eval_op;
            this->expr_ = ifexpr->test();


            /* when control arrives at .cont_ will have:
             *   .value_ -> result of evaluating ifexpr->test()
             */
            this->stack_ = VsmStackFrame::push1(mm, this->stack_, ifexpr_boxed, cont_);

            /* .stack_:
             *   frame
             *     [0] = ifexpr (boxed expression)
             */

            this->cont_ = &VsmOps::complete_ifexpr_op;
        }

        void
        VirtualSchematikaMachine::do_complete_ifexpr_op()
        {
            using xo::scm::IfExpr;

            scope log(XO_DEBUG(true));

            /*
             * - value: contains result of evaluating test condition of if-expr
             * - stack: top frame has 1 slot, holds (boxed) if-expr itself
             */
            assert(value_.get());
            assert(stack_.get());
            assert(env_.get());

            gp<Boolean> test_value = gp<Boolean>::from(value_);

            if (test_value.get()) {
                gp<VsmStackFrame> sp0 = this->stack_;

                bp<IfExpr> ifexpr = IfExpr::from(ExpressionBoxed::from((*sp0)[0])->contents());

                assert(ifexpr.get());

                this->pc_ = &VsmOps::eval_op;

                if (test_value->value()) {
                    this->expr_ = ifexpr->when_true();
                } else {
                    if (ifexpr->when_false()) {
                        this->expr_ = ifexpr->when_false();
                    } else {
                        /* 1-sided if-expr; evaluate to false */
                        this->expr_ = Constant<bool>::make(false);
                    }
                }

                this->stack_ = sp0->parent();
                this->cont_ = sp0->continuation();
            } else {
                std::string err = tostr("expect boolean value for result of if-expr test", xtag("value", test_value));

                this->value_ = nullptr;
                this->error_ = SchematikaError(err);

                /* note: poor man's exception */
                this->pc_ = nullptr;
                this->cont_ = nullptr;
            }
        }

        void
        VirtualSchematikaMachine::eval_sequence_op()
        {
            using xo::scm::Sequence;

            scope log(XO_DEBUG(true));

            gc::IAlloc * mm = flyweight_.object_mm_;

            /** must promote bp<Sequence> -> gp<ExpressionBoxed> **/
            gp<ExpressionBoxed> seq_boxed = ExpressionBoxed::make(mm, expr_);
            bp<Sequence> seq = Sequence::from(expr_);

            assert(seq.get());
            assert(env_.get());

            this->pc_ = &VsmOps::eval_op;

            if (seq->size() == 0) {
                /* for 0-size sequence, invent an expression */
                this->expr_ = Constant<bool>::make(false);
            } else {
                this->expr_ = (*seq)[0];
            }

            if (seq->size() > 1) {
                /* remainder */

                gp<Integer> next = Integer::make(mm, 1);

                /* when control arrives at .cont_ will have:
                 *   .value_ -> result of evaluating last expr in seq
                 */
                this->stack_ = VsmStackFrame::push2(mm, stack_, seq_boxed, next, cont_);

                /* .stack_:
                 *   frame
                 *     [0] = seq (boxed sequence)
                 *     [1] = next (index of next seq member to evaluate)
                 *   ..
                 */

                this->cont_ = &VsmOps::complete_sequence_op;
            } else {
                /* sequence completes when expr_ evaluated
                 * -> proceed with o.g. cont_
                 */
            }
        }

        void
        VirtualSchematikaMachine::do_complete_sequence_op()
        {
            using xo::scm::Sequence;

            scope log(XO_DEBUG(true));

            /* - stack: top frame has 2 slots:
             *    [0] : seq (boxed Sequence)
             *    [1] : next (index of next seq element to eval
             */

            assert(value_.get());
            assert(stack_.get());

            gp<VsmStackFrame> sp0 = this->stack_;

            assert(sp0->size() == 2);

            bp<Sequence> seq = Sequence::from(ExpressionBoxed::from((*sp0)[0])->contents());
            gp<Integer> next_obj = Integer::from((*sp0)[1]);
            size_t i_next = next_obj->value();

            assert(i_next < seq->size());

            this->pc_ = &VsmOps::eval_op;
            this->expr_ = (*seq)[i_next];

            if (i_next + 1 == seq->size()) {
                /* last member of sequence -> tail call optimization */
                this->stack_ = sp0->parent();
                this->cont_ = sp0->continuation();
            } else {
                /* we can modify next_obj in place,
                 * since it's unique to frame sp0
                 */
                next_obj->assign_value(i_next + 1);
                this->cont_ = &VsmOps::complete_sequence_op;
            }
        }

        void
        VirtualSchematikaMachine::eval_apply_op()
        {
            /* strategy:
             * 1. calling sequence will involve two stack frames.
             *    1a. the outer frame will hold 'final evaluated arguments'
             *        to the called function.  When control transfers to that
             *        function, this frame will be at the top of stack_
             *    1b. innert frame will be used by eval_apply_op() and
             *        helper do_eval_collect_args() to evaluate function
             *        arguments, and populate the outer frame.
             */

            using xo::scm::Apply;

            scope log(XO_DEBUG(true));

            gc::IAlloc * mm = flyweight_.object_mm_;

            /** must promote bp<Apply> -> gp<ExpressionBoxed> **/
            gp<ExpressionBoxed> apply_boxed = ExpressionBoxed::make(mm, expr_);
            bp<Apply> apply = Apply::from(expr_);

            assert(apply.get());

            size_t n = apply->n_arg() + 1;

            /* reminder: argument 0 refers to the function being called */
            gp<Integer> targetarg = Integer::make(mm, 0);

            /* outer frame */
            gp<VsmStackFrame> argstack = VsmStackFrame::make(mm, stack_, n, cont_);

            /* scratch frame during call sequence.
             * probably collect->cont_ will not be used?
             */
            gp<VsmStackFrame> collect = VsmStackFrame::push2(mm,
                                                             argstack,
                                                             apply_boxed,
                                                             targetarg,
                                                             &VsmOps::complete_evalargs_op);

            this->pc_ = &VsmOps::eval_op;
            this->expr_ = apply->fn();
            this->stack_ = collect;
            this->cont_ = &VsmOps::complete_evalargs_op;
        }

        void
        VirtualSchematikaMachine::do_complete_evalargs_op()
        {
            using xo::scm::Apply;

            scope log(XO_DEBUG(true));

            /* - stack: top frame has 2 slots
             *    [0] : apply (boxed Apply)
             *    [1] : targetarg index of next evaluated argument to deliver.
             *          (to corresponding slot in 2nd frame)
             * - 2nd frame has n slots, where n = #of arguments at this site
             *    [0] : actual #0
             *    ..
             *    [targetarg-1] : actual #{targetarg-1}
             */

            assert(value_.get());
            assert(stack_.get());

            gp<VsmStackFrame> sp0 = this->stack_;

            assert(sp0.get());
            assert(sp0->size() == 2);

            bp<Apply> apply = Apply::from(ExpressionBoxed::from((*sp0)[0])->contents());
            assert(apply.get());
            gp<Integer> targetarg_obj = Integer::from((*stack_)[1]);
            size_t targetarg = targetarg_obj->value();

            /* note: apply->n_arg() doesn't count function itself */
            assert(targetarg < apply->n_arg() + 1);

            gp<VsmStackFrame> argstack = sp0->parent();

            assert(argstack.get());

            /* storing actual parameter in its correct position in call stackframe */
            (*argstack)[targetarg] = value_;

            ++targetarg;

            if (targetarg < apply->n_arg() + 1) {
                /*
                 * arguments 0 .. #targetarg-1 already present in argstack
                 * arguments #targetarg .. #n still need to be evaluated
                 */

                /* ok to update in place, since Integer in sp0 is unique */
                targetarg_obj->assign_value(targetarg);

                rp<Expression> targetexpr = apply->lookup_arg(targetarg - 1);

                this->pc_ = &VsmOps::eval_op;
                this->expr_ = targetexpr;
                assert(this->stack_.get() == sp0.get());
                this->cont_ = &VsmOps::complete_evalargs_op;
            } else {
                /* all args evaluated: proceed to invoke evaluated function */

                this->pc_ = &VsmOps::apply_op;
                this->expr_ = nullptr;
                this->stack_ = argstack;
                /* unnecessary - will actually be set by apply_op() */
                this->cont_ = argstack->continuation();
            }
        }

        void
        VirtualSchematikaMachine::apply_op()
        {
            scope log(XO_DEBUG(true));

            auto mm = flyweight_.object_mm_;

            // NOTE: Closures will have special handling.
            //       Could alternatively forward the whole problem
            //       (along with VSM state) to procedure implementation

            /* stack: top frame has n slots for procedure with n canonical args */

            gp<VsmStackFrame> sp0 = stack_;

            assert(sp0->size() > 0);

            gp<Procedure> fn = Procedure::from((*sp0)[0]);

            if (fn->n_args() + 1 != sp0->size()) {
                throw std::runtime_error(tostr("VirtualSchematikaMachine::apply_op:"
                                               " argument mismatch in apply"
                                               ": k arguments supplied where n expected",
                                               xtag("k", sp0->size() - 1),
                                               xtag("n", fn->n_args())));
            }

            /* todo:
             *  check function signature?
             *  should have been guaranteed by expression parser,
             *  but complications in interactive session when variables redefined.
             */

            gp<Object> retval = fn->apply_nocheck(mm, sp0->argv());

            this->pc_ = this->cont_;
            this->stack_ = sp0->parent();
            this->value_ = retval;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end VirtualSchematikaMachine.cpp */
