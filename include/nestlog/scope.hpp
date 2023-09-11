/* @file scopẹhpp */

#pragma once

#include "log_state.hpp"
#include "tostr.hpp"
#include "tag.hpp"

#include <stdexcept>
#include <cstdint>
#include <memory>   // for std::unique_ptr

namespace xo {

    template <typename ChartT, typename Traits>
    class state_impl;

    /* throw exception if condition not met*/
#  define XO_EXPECT(f,msg) if(!(f)) { throw std::runtime_error(msg); }
    /* establish scope using current function name */
#  define XO_SCOPE(name) xo::scope name(__FUNCTION__)
    /* like XO_SCOPE(name),  but also set enabled flag */
#  define XO_SCOPE2(name, debug_flag) xo::scope name(__FUNCTION__, debug_flag)
#  define XO_SCOPE_DISABLED(name) xo::scope name(__FUNCTION__, false)
#  define XO_STUB() { XO_SCOPE(logr); logr.log("STUB"); }

    /* nesting logger
     *
     * Use:
     *   using xo::scope;
     *
     *   void myfunc() {
     *     XO_SCOPE(log);  //or scope x("myfunc")
     *     log(a,b,c);
     *     anotherfunc();
     *     log(d,e,f);
     *   }
     *
     *   void anotherfunc() {
     *     XO_SCOPE(x); // or scope x("anotherfunc")
     *     x.log(y);
     *   }
     *
     * or:
     *   void myfunc() {
     *     bool log_flag = true;
     *     XO_SCOPE2(log, log_flag);    // create local variable 'log'
     *     log && log(a,b,c);           // log iff enabled
     *     log.end_scope();             // optional protection against compiler destroying 'log' early
     *   }
     *
     * output like:
     *    +myfunc:
     *     a,b,c
     *     +anotherfunc:
     *      y
     *     -anotherfunc:
     *     d,e,f
     *    -myfunc:
     */
    template <typename CharT, typename Traits = std::char_traits<CharT>>
    class basic_scope {
    public:
        using state_impl_type = state_impl<CharT, Traits>;

    public:
        basic_scope(std::string_view name1);
        basic_scope(std::string_view name1, bool enabled_flag);
        basic_scope(std::string_view name1, std::string_view name2, bool enabled_flag);
        ~basic_scope();

        bool enabled() const { return !finalized_; }

        operator bool() const { return this->enabled(); }

        /* report current nesting level */
        std::uint32_t nesting_level() const;

        template<typename... Tn>
        bool log(Tn&&... rest) {
            if(this->finalized_) {
                throw std::runtime_error("basic_scope: attempt to use finalized scope");
            } else {
                state_impl_type * logstate = require_indent_thread_local_state();

                /* log to per-thread stream to prevent data races */
                tosn(logstate2stream(logstate), rest...);

                this->flush2clog(logstate);
            }

            return true;
        } /*log*/

        template<typename... Tn>
        bool operator()(Tn&&... rest) { return this->log(rest...); }

        /* call once to end scope before dtor */
        void end_scope();

    private:
        /* establish stream for logging;  use thread-local storage for threadsafetỵ
         * stream, if recycled (ịẹ after 1st call to scopẹlog() from a particular thread),
         * will be in 'reset-to-beginning of buffer' statẹ
         */
        static state_impl_type * require_indent_thread_local_state();

        /* establish logging state;  use thread-local storage for threadsafety */
        static state_impl_type * require_thread_local_state();

        /* retrieve permanently-associated ostream for logging-state */
        static std::ostream & logstate2stream(state_impl_type * logstate);

        /* write collected output to std::clog,  or chosen streambuf */
        void flush2clog(state_impl_type * logstate, std::streambuf * p_sbuf = std::clog.rdbuf());

    private:
        /* keep logging state separately for each thread */
        static thread_local std::unique_ptr<state_impl_type> s_threadlocal_state;

        /* name of this scope (part 1) */
        std::string_view name1_ = "<name1>";
        /* name of this scope (part 2) */
        std::string_view name2_ = "::<name2>";
        /* set once per scope .finalized=true <-> logging disabled */
        bool finalized_ = false;
    }; /*basic_scope*/

    template <typename CharT, typename Traits>
    basic_scope<CharT, Traits>::basic_scope(std::string_view fn1,
                                            std::string_view fn2,
                                            bool enabled_flag)
        : name1_(fn1),
          name2_(fn2),
          finalized_(!enabled_flag)
    {
        if(enabled_flag) {
            state_impl_type * logstate = basic_scope::require_thread_local_state();

            logstate->preamble(this->name1_, this->name2_);
            logstate->flush2sbuf(std::clog.rdbuf());

            ///* next call to scope::log() can reset to beginning of buffer space */
            //logstate->ss().seekp(0);

            logstate->incr_nesting();
        }
    } /*ctor*/

    template <typename CharT, typename Traits>
    basic_scope<CharT, Traits>::basic_scope(std::string_view fn1, bool enabled_flag)
        : basic_scope(fn1, "", enabled_flag)
    {}

    template <typename CharT, typename Traits>
    basic_scope<CharT, Traits>::basic_scope(std::string_view fn)
        : basic_scope(fn, true /*enabled_flag*/)
    {}

    template <typename CharT, typename Traits>
    basic_scope<CharT, Traits>::~basic_scope() {
        if(!this->finalized_)
            this->end_scope();
    } /*dtor*/

    template <typename CharT, typename Traits>
    thread_local std::unique_ptr<state_impl<CharT, Traits>>
    basic_scope<CharT, Traits>::s_threadlocal_state;

    template <typename CharT, typename Traits>
    std::uint32_t
    basic_scope<CharT, Traits>::nesting_level() const {
        return require_thread_local_state()->nesting_level();
    } /*nesting_level*/

    template <typename CharT, typename Traits>
    basic_scope<CharT, Traits>::state_impl_type *
    basic_scope<CharT, Traits>::require_indent_thread_local_state()
    {
        state_impl_type * local_state = require_thread_local_state();

        local_state->reset_stream();
        local_state->indent(' ' /*pad_char*/);

        return local_state;
    } /*require_thread_local_stream*/

    template <typename CharT, typename Traits>
    basic_scope<CharT, Traits>::state_impl_type *
    basic_scope<CharT, Traits>::require_thread_local_state()
    {
        if(!s_threadlocal_state) {
            s_threadlocal_state.reset(new state_impl_type());
        }

        return s_threadlocal_state.get();
    } /*require_thread_local_state*/

    template <typename CharT, typename Traits>
    std::ostream &
    basic_scope<CharT, Traits>::logstate2stream(state_impl_type * logstate)
    {
        return logstate->ss();
    } /*logstate2stream*/

    template <typename CharT, typename Traits>
    void
    basic_scope<CharT, Traits>::flush2clog(state_impl_type * logstate,
                                           std::streambuf * p_sbuf)
    {
        logstate->flush2sbuf(p_sbuf);
    } /*flush2clog*/

    template <typename CharT, typename Traits>
    void
    basic_scope<CharT, Traits>::end_scope()
    {
        if(!this->finalized_) {
            this->finalized_ = true;

            state_impl_type * logstate
                = basic_scope::require_thread_local_state();

            logstate->decr_nesting();

            logstate->postamble(this->name1_, this->name2_);
            logstate->flush2sbuf(std::clog.rdbuf());
        }
    } /*end_scope*/


    using scope = basic_scope<char>;

} /*namespace xo*/

/* end scope.hpp */
