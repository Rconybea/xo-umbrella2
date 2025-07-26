/* @file expression.text.cppreference
 *
 * author: Roland Conybeare, Jul 2025
 */

#include "xo/expression/typeinf/type_unifier.hpp"
#include "xo/reflect/Reflect.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace ut {
        // rehearser copied from xo_tokenizer/utest/tokenizer.test.cpp

        /** Two-pass test harness.
         *
         *   First pass - verify test assertions.
         *   Second pass only if first pass failed.
         *   On second pass, enable verbose logging
         **/
        struct rehearser {
            rehearser(std::uint32_t att = 0) : attention_{att} {}

            /* expect at most one iterator to exist per TestRehearser instance **/
            struct iterator {
                explicit iterator(rehearser* parent) : parent_{parent} {}

                iterator& operator++();
                std::uint32_t operator*() { return parent_->attention_; }

                bool operator==(const iterator& ix2) const {
                    return (parent_ == ix2.parent_);
                }

                rehearser* parent_ = nullptr;
                std::uint32_t attention_ = 0;

            };

            bool is_first_pass() const { return attention_ == 0; }
            bool is_second_pass() const { return attention_ == 1; }
            bool enable_debug() const { return is_second_pass(); }

            iterator begin() { return iterator(this); }
            iterator end()   { return iterator(nullptr); }

        public:
            /** pass number: 0 or 1 **/
            std::uint32_t attention_ = 0;
            /** @brief set to true when test starts; false if first pass fails **/
            bool ok_flag_ = true;
        };

        auto rehearser::iterator::operator++() -> iterator&
        {
            if (parent_)
                ++(parent_->attention_);

            if (parent_->ok_flag_ && (parent_->attention_ == 1)) {
                /* skip 2nd pass */
                ++(parent_->attention_);
            }

            if (parent_->attention_ == 2)
                parent_ = nullptr;

            return *this;
        }

        /* use this instead of REQUIRE(expr) in context of a test_rehearser
         * REQUIRE(true) in first pass so we count assertions
         */
#      define REHEARSE(rehearser, expr)                    \
        if (rehearser.is_first_pass()) {                   \
            REQUIRE(true);                                 \
            bool _f = (expr);                              \
            rehearser.ok_flag_ = rehearser.ok_flag_ && _f; \
        } else {                                           \
            REQUIRE(expr);                                 \
        }

        /* note: trivial REQUIRE() call in else branch bc we still want
         *       catch2 to count assertions when verification succeeds
         */
#    define REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr) \
        if (catch_flag) {                                \
            REQUIRE((expr));                             \
        } else {                                         \
            REQUIRE(true);                               \
            ok_flag &= (expr);                           \
        }

#    define REQUIRE_ORFAIL(ok_flag, catch_flag, expr)    \
        REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr);    \
        if (!ok_flag)                                    \
            return ok_flag


        using xo::scm::type_unifier;
        using xo::scm::TypeBlueprint;
        using xo::scm::unify_result;
        using xo::scm::type_ref;
        using xo::scm::type_var;
        using xo::reflect::Reflect;
        using xo::reflect::TypeDescr;

        namespace {
            struct testcase_ufy {
                /* using lambda's here to ensure don't share state between test loop iterations below */
                std::function<rp<TypeBlueprint>()> lhs_;
                std::function<rp<TypeBlueprint>()> rhs_;

                bool expect_unify_ok_ = false;
                type_var expect_unify_id_;
                bool expect_unify_concrete_ = false;
                std::string expect_concrete_typename_;
                bool expect_unify_variable_ = false;
            };

            std::vector<testcase_ufy>
            s_testcase_v = {
                /* unify two variables */
                {
                    .lhs_ = [](){ return TypeBlueprint::typevar(type_var::from_chars("a")); },
                    .rhs_ = [](){ return TypeBlueprint::typevar(type_var::from_chars("b")); },
                    .expect_unify_ok_ = true,
                    .expect_unify_id_{type_var::from_chars("a")},
                    .expect_unify_concrete_ = false,
                    .expect_concrete_typename_ = "",
                    .expect_unify_variable_ = true,
                },
                /* unify two variables (different order) */
                {
                    .lhs_ = [](){ return TypeBlueprint::typevar(type_var::from_chars("b")); },
                    .rhs_ = [](){ return TypeBlueprint::typevar(type_var::from_chars("a")); },
                    .expect_unify_ok_ = true,
                    .expect_unify_id_{type_var::from_chars("a")},
                    .expect_unify_concrete_ = false,
                    .expect_concrete_typename_ = "",
                    .expect_unify_variable_ = true,
                },
                /* unify a variable with a concrete type */
                {
                    .lhs_ = [](){ return TypeBlueprint::typevar(type_var::from_chars("a")); },
                    .rhs_ = [](){ return TypeBlueprint::make(type_ref(type_var::from_chars("b"),
                                                                      Reflect::require<std::int64_t>())); },
                    .expect_unify_ok_ = true,
                    .expect_unify_id_{type_var::from_chars("a")},
                    .expect_unify_concrete_ = true,
                    .expect_concrete_typename_ = "long int",
                    .expect_unify_variable_ = false,
                },
                /* same, but reverse order */
                {
                    .lhs_ = [](){ return TypeBlueprint::make(type_ref(type_var::from_chars("a"),
                                                                      Reflect::require<std::int64_t>())); },
                    .rhs_ = [](){ return TypeBlueprint::typevar(type_var::from_chars("b")); },
                    .expect_unify_ok_ = true,
                    .expect_unify_id_{type_var::from_chars("a")},
                    .expect_unify_concrete_ = true,
                    .expect_concrete_typename_ = "long int",
                    .expect_unify_variable_ = false,
                },
                /* matching concrete types */
                {
                    .lhs_ = [](){ return TypeBlueprint::make(type_ref(type_var::from_chars("a"),
                                                                      Reflect::require<bool>())); },
                    .rhs_ = [](){ return TypeBlueprint::make(type_ref(type_var::from_chars("b"),
                                                                      Reflect::require<bool>())); },
                    .expect_unify_ok_ = true,
                    .expect_unify_id_{type_var::from_chars("a")},
                    .expect_unify_concrete_ = true,
                    .expect_concrete_typename_ = "bool",
                    .expect_unify_variable_ = false,
                },

                /* conflicting concrete types */
                {
                    .lhs_ = [](){ return TypeBlueprint::make(type_ref(type_var::from_chars("a"),
                                                                      Reflect::require<bool>())); },
                    .rhs_ = [](){ return TypeBlueprint::make(type_ref(type_var::from_chars("b"),
                                                                      Reflect::require<char>())); },
                    .expect_unify_ok_ = false,
                    // remainder ignored
                    .expect_unify_id_{},
                    .expect_unify_concrete_{},
                    .expect_concrete_typename_{},
                    .expect_unify_variable_{},
                }
            };
        }

        TEST_CASE("unifier", "[type-unification]")
        {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const testcase_ufy & testcase = s_testcase_v[i_tc];

                rehearser rh;
                CHECK(rh.ok_flag_ == true);

                for (auto _ : rh) {
                    /* this loop problematic because iterations aren't independent:
                     * TypeBlueprint instances modified in place by unification
                     */

                    scope log(XO_DEBUG2(rh.enable_debug(), "unifier"));

                    auto lhs = testcase.lhs_();
                    auto rhs = testcase.rhs_();

                    log && log(xtag("i_tc", i_tc),
                               xtag("lhs", lhs),
                               xtag("rhs", rhs));

                    type_unifier unifier;

                    auto ur = unifier.unify(lhs, rhs);

                    log && log(xtag("ur", ur));

                    REHEARSE(rh, ur.success_ == testcase.expect_unify_ok_);
                    if (testcase.expect_unify_ok_) {
                        REHEARSE(rh, ur.unified_.get() != nullptr);
                        if (ur.unified_) {
                            REHEARSE(rh, ur.unified_->id() == testcase.expect_unify_id_);
                            REHEARSE(rh, ur.unified_->is_concrete() == testcase.expect_unify_concrete_);
                            if (ur.unified_->is_concrete()) {
                                REHEARSE(rh, ur.unified_->td()->canonical_name() == testcase.expect_concrete_typename_);
                            }
                            REHEARSE(rh, ur.unified_->is_variable() == testcase.expect_unify_variable_);
                        }
                    }
                    //REHEARSE(rh, ur.error_description_ == testcase.expect_unify_error_);
                }
            }
        }


    } /*namespace ut*/
} /*namespace xo*/
