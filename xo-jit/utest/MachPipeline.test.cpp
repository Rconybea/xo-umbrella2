/* @file MachPipeline.test.cpp */

#include "xo/jit/MachPipeline.hpp"
#include "xo/expression/Primitive.hpp"
#include "xo/ratio/ratio.hpp"
#include "xo/ratio/ratio_reflect.hpp"
#include "xo/reflect/reflect_struct.hpp"
#include "xo/indentlog/scope.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::jit::MachPipeline;
    using xo::scm::make_apply;
    using xo::scm::make_var;
    using xo::scm::make_primitive;
    using xo::scm::llvmintrinsic;
    using xo::scm::Expression;
    using xo::scm::Lambda;
    using xo::scm::exprtype;
    using xo::reflect::Reflect;
    using xo::reflect::reflect_struct;
    using xo::ref::brw;
    using std::cerr;
    using std::endl;

    namespace ut {

        /* abstract syntax tree for a function:
         *   def root4(x :: double) { sqrt(sqrt(x)); }
         */
        rp<Expression>
        root4_ast() {
            auto sqrt = make_primitive("sqrt",
                                       ::sqrt,
                                       false /*!explicit_symbol_def*/,
                                       llvmintrinsic::fp_sqrt);
            auto x_var = make_var("x", Reflect::require<double>());
            auto call1 = make_apply(sqrt, {x_var});
            auto call2 = make_apply(sqrt, {call1});

            auto fn_ast = make_lambda("root4",
                                      {x_var},
                                      call2,
                                      nullptr /*parent_env*/);

            return fn_ast;
        }

        /* abstract syntax tree for a function:
         *   def twice(f :: double->double, x :: double) { f(f(x)); }
         */
        rp<Expression>
        root_2x_ast() {
            auto root = make_primitive("sqrt",
                                       ::sqrt,
                                       false /*!explicit_symbol_def*/,
                                       llvmintrinsic::fp_sqrt);

            /* def twice(f :: double->double, x :: double) { f(f(x)) } */
            auto f_var = make_var("f", Reflect::require<double (*)(double) noexcept>());
            auto x_var = make_var("x", Reflect::require<double>());
            auto call1 = make_apply(f_var, {x_var}); /* (f x) */
            auto call2 = make_apply(f_var, {call1}); /* (f (f x)) */

            /*   def twice(f :: double->double, x :: double) { f(f(x)); } */
            auto twice = make_lambda("twice",
                                     {f_var, x_var},
                                     call2,
                                     nullptr /*parent_env*/);

            auto x2_var = make_var("x2", Reflect::require<double>());
            auto call3 = make_apply(twice, {root, x2_var});

            /*   def root4(x2 :: double) {
             *     def twice(f :: double->double, x :: double) { f(f(x)); }};
             *     twice(sqrt, x2)
             *   }
             */
            auto fn_ast = make_lambda("root_2x",
                                      {x2_var},
                                      call3,
                                      nullptr /*parent_env*/);

            return fn_ast;
        }

        struct TestCase {
            rp<Expression> (*make_ast_)();
            /* each pair is (input, output) for function double->double */
            std::vector<std::pair<double,double>> call_v_;
        };

        static std::vector<TestCase>
        s_testcase_v = {
            {&root4_ast,
             {std::make_pair(1.0, 1.0),
              std::make_pair(16.0, 2.0),
              std::make_pair(81.0, 3.0)}},
            {&root_2x_ast,
             {std::make_pair(1.0, 1.0),
              std::make_pair(16.0, 2.0),
              std::make_pair(81.0, 3.0)}}
        };

        /** testcase root_ast tests:
         *  - nested function call
         *
         *  testcase root_2x_ast relies on:
         *  - lambda in function position
         *  - argument with function type
         **/
        TEST_CASE("machpipeline.fptr", "[llvm][llvm_fnptr]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.machpipeline.fptr"));
            //log && log("(A)", xtag("foo", foo));


            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                /** can't share jit across examples,
                 *  until we fix treatment of primitives:
                 *  now that we build a wrapper for each primitive,
                 *  need some bookkeeping to avoid trying to build
                 *  the same wrapper twice.
                 **/
                auto jit = MachPipeline::make();

                TestCase const & testcase = s_testcase_v[i_tc];

                INFO(tostr(xtag("i_tc", i_tc)));

                auto ast = (*testcase.make_ast_)();

                REQUIRE(ast.get());

                log && log(xtag("ast", ast));

                REQUIRE(ast->extype() == exprtype::lambda);

                brw<Lambda> fn_ast = Lambda::from(ast);

                llvm::Value * llvm_ircode = jit->codegen_toplevel(fn_ast);

                /* TODO: printer for llvm::Value* */
                if (llvm_ircode) {
                    /* note: llvm:errs() is 'raw stderr stream' */
                    cerr << "llvm_ircode:" << endl;
                    llvm_ircode->print(llvm::errs());
                    cerr << endl;
                } else {
                    cerr << "code generation failed"
                         << xtag("fn_ast", fn_ast)
                         << endl;
                }

                REQUIRE(llvm_ircode);

                jit->machgen_current_module();

                /** lookup compiled function pointer in jit **/
                auto llvm_addr = jit->lookup_symbol(fn_ast->name());

                if (!llvm_addr) {
                    cerr << "ex2: lookup: symbol not found"
                         << xtag("symbol", fn_ast->name())
                         << endl;
                } else {
                    cerr << "ex2: lookup: symbol found"
                         << xtag("llvm_addr", llvm_addr.get().getValue())
                         << xtag("symbol", fn_ast->name())
                         << endl;
                }

                auto fn_ptr = llvm_addr.get().toPtr<double(*)(double)>();

                REQUIRE(fn_ptr);

                for (std::size_t j_call = 0, n_call = testcase.call_v_.size(); j_call < n_call; ++j_call) {
                    double input = testcase.call_v_[j_call].first;
                    double expected = testcase.call_v_[j_call].second;

                    INFO(tostr(xtag("j_call", j_call), xtag("input", input), xtag("expected", expected)));

                    auto actual = (*fn_ptr)(input);

                    REQUIRE(actual == expected);
                }
            }
        } /*TEST_CASE(machpipeline.fptr)*/

        TEST_CASE("machpipeline.wrap", "[llvm][llvm_closure]") {
            constexpr bool c_debug_flag = true;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.machpipelin.wrap"));

            auto jit = MachPipeline::make();

            auto root = make_primitive("sqrt",
                                       ::sqrt,
                                       false /*!explicit_symbol_def*/,
                                       llvmintrinsic::fp_sqrt);

            llvm::Value * llvm_ircode
                = jit->codegen_primitive_wrapper(root, *(jit->llvm_current_ir_builder()));

            /* TODO: printer for llvm::Value* */
            if (llvm_ircode) {
                /* note: llvm:errs() is 'raw stderr stream' */
                cerr << "llvm_ircode for primitive wrapper:" << endl;
                llvm_ircode->print(llvm::errs());
                cerr << endl;
            } else {
                cerr << "code generation failed"
                     << xtag("root", root)
                     << endl;
            }

            REQUIRE(llvm_ircode);

            std::string wrapper_name = std::string("w.") + root->name();

            jit->machgen_current_module();

            auto llvm_addr = jit->lookup_symbol(wrapper_name);

            bool llvm_addr_flag = static_cast<bool>(llvm_addr);

            if (!llvm_addr_flag) {
                cerr << "ex2: lookup: symbol not found"
                     << xtag("symbol", wrapper_name)
                     << endl;
            } else {
                cerr << "ex2: lookup: symbol found"
                     << xtag("llvm_addr", llvm_addr.get().getValue())
                     << xtag("symbol", wrapper_name)
                     << endl;
            }

            REQUIRE(llvm_addr_flag);

            auto fn_ptr = llvm_addr.get().toPtr<double(*)(void*, double)>();

            REQUIRE(fn_ptr);

            auto actual = (*fn_ptr)(nullptr, 4.0);

            REQUIRE(actual == 2.0);
        }

        rp<Lambda>
        make_ratio() {
            auto make_ratio_impl = make_primitive("make_ratio_impl",
                                                  xo::ratio::make_ratio<int, int>,
                                                  true /*explicit_symbol_def*/,
                                                  llvmintrinsic::invalid);
            REQUIRE(make_ratio_impl.get());
            REQUIRE(make_ratio_impl->explicit_symbol_def());

            /* jit-prepared library:
             * 1. *uses* make_ratio_impl
             * 2. *provides* make_ratio  (can do jit->lookup_symbol("make_ratio"))
             */
            auto n_var = make_var("n", Reflect::require<int>());
            auto d_var = make_var("d", Reflect::require<int>());
            auto call1 = make_apply(make_ratio_impl, {n_var, d_var}); /*make_ratio(n,d)*/

            auto make_ratio = make_lambda("make_ratio",
                                          {n_var, d_var},
                                          call1,
                                          nullptr /*parent_env*/);

            return make_ratio;
        }

        TEST_CASE("machpipeline.struct", "[llvm][llvm_struct]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.machpipeline.struct"));
            //log && log("(A)", xtag("foo", foo));

            auto jit = MachPipeline::make();

            /* let's reflect xo::ratio::ratio<int> */

            using ratio_type = xo::ratio::ratio<int>;

            auto struct_td = reflect_struct<ratio_type>();

            REQUIRE(struct_td);

            // ----- build AST -----

            auto fn_ast = make_ratio();

            // ----- convert AST -> llvm IR datastructure -----

            llvm::Value * llvm_ircode = jit->codegen_toplevel(fn_ast);

            /* TODO: printer for llvm::Value* */
            if (llvm_ircode) {
                /* note: llvm:errs() is 'raw stderr stream' */
                cerr << "llvm_ircode:" << endl;
                llvm_ircode->print(llvm::errs());
                cerr << endl;
            } else {
                cerr << "code generation failed"
                     << xtag("fn_ast", fn_ast)
                     << endl;
            }

            REQUIRE(llvm_ircode);

            // ----- inspect alignment -----

            llvm::StructType * struct_llvm_type
                = static_cast<llvm::StructType *>(jit->codegen_type(struct_td));

            auto struct_layout = jit->data_layout().getStructLayout(struct_llvm_type);

            log && log(xtag("struct-size", struct_layout->getSizeInBytes()),
                       xtag("struct-alignment", struct_layout->getAlignment().value()));
            for (int i = 0, n = struct_llvm_type->getNumElements(); i < n; ++i) {
                llvm::TypeSize llvm_tz = struct_layout->getElementOffset(i);
                auto offset = reinterpret_cast<uint64_t>(struct_td->struct_member(i).get_member_tp(nullptr).address());

                log && log(xtag("i", i),
                           xtag("name(c++)", struct_td->struct_member(i).member_name()),
                           xtag("type(c++)", struct_td->struct_member(i).get_member_td()->short_name()),
                           xtag("offset(c++)", offset),
                           xtag("offset(llvm)", llvm_tz.getKnownMinValue()));

                REQUIRE(offset == llvm_tz.getKnownMinValue());
            }

            // ----- generate JIT machine code -----

            jit->machgen_current_module();

            log && log("execution session after codegen:");
            //log && log(jit->xsession());  // segfaults
            jit->dump_execution_session();

            // ----- verify: lookup symbol

            /** lookup compiled function pointer in jit **/
            auto llvm_addr = jit->lookup_symbol(fn_ast->name());

            log && log("execution session after lookup attempt:");
            jit->dump_execution_session();

            if (!llvm_addr) {
                cerr << "ex2: lookup: symbol not found"
                     << xtag("symbol", fn_ast->name())
                     << endl;
            } else {
                cerr << "ex2: lookup: symbol found"
                     << xtag("llvm_addr", llvm_addr.get().getValue())
                     << xtag("symbol", fn_ast->name())
                     << endl;
            }

            auto fn_ptr = llvm_addr.get().toPtr<ratio_type(*)(int,int)>();

            REQUIRE(fn_ptr);

            // ---- invoke compiled function -----

            auto value = (*fn_ptr)(2, 3);

            log && log(xtag("value.num", value.num()),
                       xtag("value.den", value.den()));

        } /*TEST_CASE(machpipeline.struct)*/
    } /*namespace ut*/
} /*namespace xo*/


/* end MachPipeline.test.cpp */
