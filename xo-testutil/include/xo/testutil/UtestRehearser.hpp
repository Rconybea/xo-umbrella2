/** @file UtestRehearser.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include <cstdint>

namespace xo {
    /** Two-pass test harness.
     *
     *   First pass - verify test assertions.
     *   Second pass [omit if first pass succeeds] - test assertions + verbose logging
     **/
    struct UtestRehearser {
        using uint32_t = std::uint32_t;

        UtestRehearser(std::uint32_t att = 0) : attention_{att} {}

        /** For a particular unit test:
         *  iterate over unit test passes.
         *
         *
         *  Expect at most one iterator to exist per TestRehearser instance
         **/
        struct iterator {
            explicit iterator(UtestRehearser * parent) : parent_{parent} {}

            iterator & operator++();
            uint32_t operator*() { return parent_->attention_; }

            bool operator==(const iterator& ix2) const {
                return (parent_ == ix2.parent_);
            }

            UtestRehearser * parent_ = nullptr;
            /** 0 for 1st pass, 1 for 2nd pass **/
            uint32_t attention_ = 0;
        };

        bool is_first_pass() const { return attention_ == 0; }
        bool is_second_pass() const { return attention_ == 1; }
        bool enable_debug() const { return is_second_pass(); }

        iterator begin() { return iterator(this); }
        iterator end()   { return iterator(nullptr); }

    public:
        /** pass number: 0 or 1 **/
        uint32_t attention_ = 0;
        /** @brief set to true when test starts; false if first pass fails **/
        bool ok_flag_ = true;
    };

    /* use this instead of REQUIRE(expr) in context of a test_rehearser */
#  define REHEARSE(rehearser, expr)                        \
    if ((rehearser).is_first_pass()) {                     \
        bool _f = (expr);                                  \
        (rehearser).ok_flag_ = (rehearser).ok_flag_ && _f; \
    } else {                                               \
        REQUIRE(expr);                                     \
    }

    /* note: trivial REQUIRE() call in else branch bc we still want
     *       catch2 to count assertions when verification succeeds
     */
#  define REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr)    \
    if (catch_flag) {                                     \
        REQUIRE((expr));                                  \
    } else {                                              \
        REQUIRE(true);                                    \
        ok_flag &= (expr);                                \
    }

#  define REQUIRE_ORFAIL(ok_flag, catch_flag, expr)   \
    REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr);     \
    if (!ok_flag)                                     \
        return ok_flag


} /*namespace xo*/

/* end UtestRehearser.hpp */
