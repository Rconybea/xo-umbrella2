/** @file verify_policy.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 *
 *  policy for a verify_ok() method's error-reporting behavior: silent, log,
 *  throw, or both.  Written first for DArenaHashMap, but subsystem-agnostic --
 *  any type with a verify_ok() (RedBlackTree, ...) can use it.
 *
 *  Lives in xo-ppsink (not the subsystem being verified) because its only
 *  dependencies are the ppsink scope logger + xo::pp::tostr.
 **/

#pragma once

#include "scope.hpp"
#include "tostr.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>

namespace xo {
    /** @brief policy for verify_ok behavior.
     *
     *  Remark: wrote this for DArenaHashMap,
     *  want to incorporate into other subsystems
     *  that provide a verify_ok() method.
     *  e.g. RedBlackTree
     **/
    struct verify_policy {
        static verify_policy log_only() {
            return verify_policy{.flags_ = 0x01};
        }
        static verify_policy throw_only() {
            return verify_policy{.flags_ = 0x02};
        }
        static verify_policy chatty() {
            return verify_policy{.flags_ = 0x03};
        }

        bool is_silent() const noexcept { return flags_ == 0; }
        bool log_flag() const noexcept { return flags_ & 0x01; }
        bool throw_flag() const noexcept { return flags_ & 0x02; }

        template<typename... Tn>
        bool report_error(xo::pp::scope & log, Tn&&... args)
        {
            using xo::pp::tostr;

            if (!this->is_silent()) {
                // TODO: consider global arena here for string
                std::string msg = tostr(std::forward<Tn>(args)...);

                if (this->log_flag()) {
                    log.retroactively_enable();
                    log(msg);
                }
                if (this->throw_flag()) {
                    throw std::runtime_error(msg);
                }
            }
            return false;
        }

        const char * c_self_ = "anonymous";
        uint8_t flags_;
    };
} /*namespace xo*/

/* end verify_policy.hpp */
