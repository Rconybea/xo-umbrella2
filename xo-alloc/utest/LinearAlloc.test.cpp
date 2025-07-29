/* @file LinearAlloc.test.cpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#include "xo/alloc/LinearAlloc.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::gc::LinearAlloc;

    namespace ut {

        namespace {
            struct testcase_alloc {
                testcase_alloc(std::size_t rz, std::size_t z)
                    : redline_z_{rz}, arena_z_{z} {}

                std::size_t redline_z_;
                std::size_t arena_z_;

            };

            std::vector<testcase_alloc>
            s_testcase_v = {
                testcase_alloc(0, 4096)
            };
        }


        TEST_CASE("linearalloc", "[alloc]")
        {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const testcase_alloc & tc = s_testcase_v[i_tc];

                auto alloc = LinearAlloc::make(tc.redline_z_, tc.arena_z_);

                REQUIRE(alloc.get());
                REQUIRE(alloc->size() == tc.arena_z_);
                REQUIRE(alloc->available() == tc.arena_z_);
                REQUIRE(alloc->allocated() == 0);
                REQUIRE(alloc->is_before_checkpoint(alloc->free_ptr()) == false);
                REQUIRE(alloc->before_checkpoint() == 0);
                REQUIRE(alloc->after_checkpoint() == 0);
            }
        }

    } /*namespace ut */
} /*namespace xo*/
