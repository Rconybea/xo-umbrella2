/* @file FunctionTdx.test.cpp */

#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/function/FunctionRepr.hpp"
#include <catch2/catch.hpp>
#include <functional>
#include <string>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;
    using xo::reflect::Metatype;
    using xo::reflect::FunctionRepr;
    using xo::reflect::FunctionTdxInfo;
    using xo::reflect::TypeDescrBase;

    namespace ut {
        TEST_CASE("function-reflect1", "[reflect]") {
            using FunctionType = double (*)(double);

            FunctionType fn = ::sqrt;

            TaggedPtr tp = Reflect::make_tp(&fn);
            //TypeDescr td = Reflect::require<std::vector<double>>();

            REQUIRE(Reflect::is_reflected<FunctionType>() == true);

            REQUIRE(tp.td()->complete_flag());
            REQUIRE(tp.address() == &fn);
            REQUIRE(tp.is_function());
            REQUIRE(tp.is_pointer() == false);
            REQUIRE(tp.is_vector() == false);
            REQUIRE(tp.is_struct() == false);
            REQUIRE(tp.td()->metatype() == Metatype::mt_function);
            REQUIRE(tp.recover_native<double (*)(double)>() == &fn);
            REQUIRE(tp.n_child() == 0); /*not a composite*/
            // REQUIRE(tp.child_td(0) == ...
            REQUIRE(tp.td()->fn_retval() == Reflect::require<double>());
            REQUIRE(tp.n_fn_arg() == 1);
            REQUIRE(tp.td()->fn_arg(0) == Reflect::require<double>());
        } /*TEST_CASE(function-reflect1)*/

        namespace {
            int twice(double x) { return static_cast<int>(2 * x); }
            unsigned bump(unsigned char x) { return x + 1u; }
        }

        /* std::function: an mt_function, its signature captured
         * (.xo-backlog/xo-reflect/issues/09)
         */
        TEST_CASE("std-function-reflects-as-mt-function", "[reflect][function]") {
            TypeDescr td = Reflect::require<std::function<int (double)>>();

            REQUIRE(td->metatype() == Metatype::mt_function);
            REQUIRE(td->is_function());
            REQUIRE(td->fn_repr() == FunctionRepr::std_function);
            REQUIRE(td->fn_retval() == Reflect::require<int>());
            REQUIRE(td->n_fn_arg() == 1);
            REQUIRE(td->fn_arg(0) == Reflect::require<double>());
            REQUIRE(td->fn_is_noexcept() == false);

            /* a function pointer: repr pointer */
            REQUIRE(Reflect::require<int (*)(double)>()->fn_repr() == FunctionRepr::pointer);
        }

        /* the representation is part of a function type's key: two
         * FunctionTdxInfos that differ only in it are different keys
         */
        TEST_CASE("function-repr-is-part-of-the-key", "[reflect][function]") {
            FunctionTdxInfo ptr_info(Reflect::require<int>(), {Reflect::require<double>()}, false);
            FunctionTdxInfo fn_info(Reflect::require<int>(), {Reflect::require<double>()}, false,
                                    FunctionRepr::std_function);

            REQUIRE(ptr_info.repr_ == FunctionRepr::pointer);   /* the 3-argument form's default */
            REQUIRE(!(ptr_info == fn_info));
            REQUIRE(std::hash<FunctionTdxInfo>{}(ptr_info) != std::hash<FunctionTdxInfo>{}(fn_info));
            REQUIRE(ptr_info.make_canonical_name() != fn_info.make_canonical_name());
        }

        /* same signature, different C++ types: two entries, and the
         * ingredient lookup still finds the pointer.  Both orders, each with
         * its own signature.  (Native function types are found by typeid or
         * canonical name, not through s_function_type_map -- only
         * ingredient-built ones enter that.)
         */
        TEST_CASE("std-function-and-pointer-intern-separately-pointer-first", "[reflect][function]") {
            TypeDescr ptr_td = Reflect::require<long (*)(short)>();
            TypeDescr fn_td = Reflect::require<std::function<long (short)>>();

            REQUIRE(ptr_td != fn_td);
            REQUIRE(TypeDescrBase::require_by_fn_info(FunctionTdxInfo(Reflect::require<long>(),
                                                                      {Reflect::require<short>()},
                                                                      false))
                    == ptr_td);
        }

        TEST_CASE("std-function-and-pointer-intern-separately-std-function-first", "[reflect][function]") {
            TypeDescr fn_td = Reflect::require<std::function<char (float)>>();
            TypeDescr ptr_td = Reflect::require<char (*)(float)>();

            REQUIRE(ptr_td != fn_td);
            REQUIRE(TypeDescrBase::require_by_fn_info(FunctionTdxInfo(Reflect::require<char>(),
                                                                      {Reflect::require<float>()},
                                                                      false))
                    == ptr_td);
        }

        /* a reference argument and a void return: as a function pointer
         * with that signature reports them
         */
        TEST_CASE("std-function-signature-with-reference-and-void", "[reflect][function]") {
            TypeDescr fn_td = Reflect::require<std::function<void (std::string const &)>>();
            TypeDescr ptr_td = Reflect::require<void (*)(std::string const &)>();

            REQUIRE(fn_td->n_fn_arg() == 1);
            REQUIRE(fn_td->fn_retval() == ptr_td->fn_retval());
            REQUIRE(fn_td->fn_arg(0) == ptr_td->fn_arg(0));
        }

        /* set or empty, read from the value at the time asked */
        TEST_CASE("function-is-set", "[reflect][function]") {
            {
                std::function<int (double)> f;
                TypeDescr td = Reflect::require<std::function<int (double)>>();

                REQUIRE(td->fn_is_set(&f) == false);
                f = [](double x) { return static_cast<int>(x); };
                REQUIRE(td->fn_is_set(&f) == true);
                f = nullptr;
                REQUIRE(td->fn_is_set(&f) == false);
            }
            {
                int (*p)(double) = nullptr;
                TypeDescr td = Reflect::require<int (*)(double)>();

                REQUIRE(td->fn_is_set(&p) == false);
                p = &twice;
                REQUIRE(td->fn_is_set(&p) == true);
            }
            {
                /* ingredient-built: a signature no test reflects natively
                 * (unsigned (*)(unsigned char)), so this entry has no
                 * native type -- the same null test applies
                 */
                TypeDescr td = TypeDescrBase::require_by_fn_info(FunctionTdxInfo(Reflect::require<unsigned>(),
                                                                                 {Reflect::require<unsigned char>()},
                                                                                 false));
                REQUIRE(td->native_typeinfo() == nullptr);

                unsigned (*p)(unsigned char) = nullptr;
                REQUIRE(td->fn_is_set(&p) == false);
                p = &bump;
                REQUIRE(td->fn_is_set(&p) == true);
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end FunctionTdx.test.cpp */
