/** @file Primitive.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "Procedure.hpp"
#include "CVector.hpp"

namespace xo {
    namespace obj {
        // TODO: consider PrimitiveInterface here

        /** @class Primitive
         *  @brief Procedure implemented natively, i.e. in c++
         *
         *  @tparam FunctionType can be:
         *  - a C-style function signature such as double(*)(double)
         *  - a std::function, such as std::function<double(double)>
         **/
        template <typename Fn>
        class PrimitiveBase : public Procedure {
        public:
            using function_type = Fn;

        public:
            explicit PrimitiveBase(Converter & cvt, Fn impl) : converter_{cvt}, impl_{std::move(impl)} {}

            // inherited from Procedure..

            virtual std::size_t n_args() const override = 0;
            virtual gp<Object> apply_nocheck(const CVector<gp<Object>> & args) override = 0;

        protected:
            Converter & converter_;
            Fn impl_;
        };

        template <typename Fn>
        class Primitive : public PrimitiveBase<Fn> {
        };

        template <Ret, Arg1, Arg2>
        class Primitive<Ret (*)(Arg1, Arg2)> : public PrimitiveBase<Ret (*)(Arg1, Arg2)> {
        public:
            using Super = PrimitiveBase<Ret (*)(Arg1, Arg2)>;
            using function_type = Ret (*)(Arg1, Arg2);

        public:
            explicit Primitive(Converter & cvt, function_type fn) : PrimitiveBase<Ret (*)(Arg1, Arg2)>{cvt, fn} {}

            // inherited from Procedure..

            virtual std::size_t n_args() const final override { return 2; }
            virtual gp<Object> apply_nocheck(const CVector<gp<Object>> & args) final override {
                /* note: args[0] will be this procedure. actual i'th function argument in args[i+1]
                 */

                args[1]
            }

        private:


        };
    }
}

/* end Primitive.hpp */
