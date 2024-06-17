/** @file Apply.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"

//#include <cstdint>

namespace xo {
    namespace ast {

        /** @class Apply
         *  @brief syntax for a function call.
         *
         *  In general we don't know function to be invoked
         *  until runtime,  depending on the nature of Expression.
         **/
        class Apply : public Expression {
        public:
            static ref::rp<Apply> make(const ref::rp<Expression> & fn,
                                       const std::vector<ref::rp<Expression>> & argv)
                {
                    return new Apply(fn, argv);
                }

            /** downcast from Expression **/
            static ref::brw<Apply> from(ref::brw<Expression> x) {
                return ref::brw<Apply>::from(x);
            }

            const ref::rp<Expression> & fn() const { return fn_; }
            const std::vector<ref::rp<Expression>> & argv() const { return argv_; }

            virtual void display(std::ostream & os) const;

        private:
            Apply(const ref::rp<Expression> & fn,
                  const std::vector<ref::rp<Expression>> & argv)
                : Expression(exprtype::apply), fn_{fn}, argv_(argv)
                {}

        private:
            /** function to invoke **/
            ref::rp<Expression> fn_;
            /** argument expressions,  in l-to-r order **/
            std::vector<ref::rp<Expression>> argv_;
        }; /*Apply*/

#ifdef NOT_USING
        namespace detail {
            /** Use:
             **   std::vector<ref::rp<Expression>>
             **/

            template <typename... Args>
            struct apply_push_args;

            template <>
            struct apply_push_args<> {
                static void push_all(std::vector<ref::rp<Expression>> * /*p_argv*/) {}
            };

            template <typename Arg1, typename... Rest>
            struct apply_push_args<Arg1, Rest...> {
                static void push_all(std::vector<ref::rp<Expression>> * p_argv,
                                     const ref::rp<Expression> & x, Rest... rest)
                    {
                        p_argv->push_back(x);
                        apply_push_args<Rest...>::push_all(p_argv, rest...);
                    };
            };
        }
#endif

        /* reminder: initializer-lists are compile-time only */
        inline ref::rp<Apply>
        make_apply(const ref::rp<Expression> & fn,
                   const std::initializer_list<ref::rp<Expression>> args) {
            std::vector<ref::rp<Expression>> argv(args);
            return Apply::make(fn, argv);
        } /*make_apply*/

    } /*namespace ast*/
} /*namespace xo*/


/** end Apply.hpp **/
