/* file SafetyToken.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

namespace xo {
    namespace web {
        /* token for cooperative compile-time threadsafety checking.
         *
         * requirements for cooperating code:
         * - token contains no state,  so in principle can be optimized away
         * - token is deliberately not copyable,  and not moveable
         * - derive from token, and make derived ctor private
         * - make method/class responsible for threadsafety a friend of token,
         *   so it can have exclusive right to create a token instance.
         * - pass token reference down stack
         *   to demonstrate ownership of protected resource,
         *   limited to the lifetime of called function.
         */
        template<typename T>
        class SafetyToken {
        public:
            SafetyToken(SafetyToken const & x) = delete;
            SafetyToken(SafetyToken && x) = delete;

            /* optionally: invoke this to "announce use of a protected resource" */
            bool verify() const { return true; }

            SafetyToken & operator=(SafetyToken const & x) = delete;
            SafetyToken & operator=(SafetyToken && x) = delete;

        protected:
            SafetyToken() = default;
        }; /*SafetyToken*/
    } /*namespace web*/
} /*namespace xo*/

/* end SafetyToken.hpp */
