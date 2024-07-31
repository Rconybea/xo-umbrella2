/* file parser.test.cpp
 *
 * author: Roland Conybeare
 */

#include "xo/parser/parser.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using parser_type = xo::scm::parser;
    using token_type = parser_type::token_type;
    using xo::scm::exprstatetype;
    using std::cerr;
    using std::endl;

    //using xo::ast::Expression;

    namespace ut {
        TEST_CASE("parser", "[parser]") {
            parser_type parser;

            parser.begin_translation_unit();

            REQUIRE(parser.stack_size() == 1);
            REQUIRE(parser.i_exstype(0)
                    == exprstatetype::expect_toplevel_expression_sequence);

            /* input:
             *   def
             */
            {
                auto r1 = parser.include_token(token_type::def());
                REQUIRE(r1.get() == nullptr);

                /* stack should be:
                 *
                 *   expect_toplevel_expression_sequence
                 *   def_0
                 *   expect_symbol
                 */
                CHECK(parser.stack_size() == 3);
                if (parser.stack_size() > 0)
                    CHECK(parser.i_exstype(0) == exprstatetype::expect_symbol);
                if (parser.stack_size() > 1)
                    CHECK(parser.i_exstype(1) == exprstatetype::def_0);
                if (parser.stack_size() > 2)
                    CHECK(parser.i_exstype(2)
                          == exprstatetype::expect_toplevel_expression_sequence);
            }

            /* input:
             *   def foo
             *      ^   ^
             *      0   1
             */
            {
                auto r2 = parser.include_token(token_type::symbol_token("foo"));

                cerr << "parser state after [def foo]" << endl;
                cerr << parser << endl;

                REQUIRE(r2.get() == nullptr);

                /* stack should be:
                 *
                 *   expect_toplevel_expression_sequence
                 *   def_1
                 */
                CHECK(parser.stack_size() == 2);
                if (parser.stack_size() > 0)
                    CHECK(parser.i_exstype(0) == exprstatetype::def_1);
                if (parser.stack_size() > 1)
                    CHECK(parser.i_exstype(1)
                          == exprstatetype::expect_toplevel_expression_sequence);

            }

            /* input:
             *   def foo :
             *          ^ ^
             *          0 1
             */
            {
                auto r3 = parser.include_token(token_type::colon());

                cerr << "parser state after [def foo :]" << endl;
                cerr << parser << endl;

                REQUIRE(r3.get() == nullptr);

                /* stack should be:
                 *
                 *   expect_toplevel_expression_sequence
                 *   def_2
                 *   expect_symbol
                 */
                CHECK(parser.stack_size() == 3);
                if (parser.stack_size() > 0)
                    CHECK(parser.i_exstype(0) == exprstatetype::expect_symbol);
                if (parser.stack_size() > 1)
                    CHECK(parser.i_exstype(1) == exprstatetype::def_2);
                if (parser.stack_size() > 2)
                    CHECK(parser.i_exstype(2)
                          == exprstatetype::expect_toplevel_expression_sequence);
            }

            /* input:
             *   def foo : footype
             *            ^       ^
             *            0       1
             */
            {
                auto r4 = parser.include_token(token_type::symbol_token("footype"));

                cerr << "parser state after [def foo : footype]" << endl;
                cerr << parser << endl;

                REQUIRE(r4.get() == nullptr);

                CHECK(parser.stack_size() == 2);

                /* stack should be:
                 *
                 *   expect_toplevel_expression_sequence
                 *   def_3
                 */
                CHECK(parser.stack_size() == 2);
                if (parser.stack_size() > 0)
                    CHECK(parser.i_exstype(0) == exprstatetype::def_3);
                if (parser.stack_size() > 1)
                    CHECK(parser.i_exstype(1)
                          == exprstatetype::expect_toplevel_expression_sequence);

                /* expecting either:
                 *   = rhs-expression
                 *   new-expression
                 */
            }

            /* input:
             *   def foo : footype =
             *                    ^ ^
             *                    0 1
             */
            {
                auto r5 = parser.include_token(token_type::singleassign());

                cerr << "parser state after [def foo : footype =]" << endl;
                cerr << parser << endl;

                REQUIRE(r5.get() == nullptr);

                CHECK(parser.stack_size() == 3);

                /* stack should be
                 *
                 *   expect_toplevel_expression_sequence
                 *   def_4
                 *   expect_expression
                 */
                CHECK(parser.stack_size() == 3);
                if (parser.stack_size() > 0)
                    CHECK(parser.i_exstype(0) == exprstatetype::expect_rhs_expression);
                if (parser.stack_size() > 1)
                    CHECK(parser.i_exstype(1) == exprstatetype::def_4);
                if (parser.stack_size() > 2)
                    CHECK(parser.i_exstype(2)
                          == exprstatetype::expect_toplevel_expression_sequence);
            }

            /* input:
             *   def foo : footype = 3.14159265
             *                      ^          ^
             *                      0          1
             */
            {
                auto r6 = parser.include_token(token_type::f64_token("3.14159265"));

                cerr << "parser state after [def foo : footype = 3.14159265]" << endl;
                cerr << parser << endl;

                REQUIRE(r6.get() != nullptr);

                CHECK(parser.stack_size() == 1);

                /* stack should be
                 *
                 *   expect_toplevel_expression_sequence
                 */
                CHECK(parser.stack_size() == 1);
                if (parser.stack_size() > 0)
                    CHECK(parser.i_exstype(0)
                          == exprstatetype::expect_toplevel_expression_sequence);
            }
        } /*TEST_CASE(parser)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end parser.test.cpp */
