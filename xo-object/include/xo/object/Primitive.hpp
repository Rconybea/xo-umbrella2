/** @file Primitive.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "Procedure.hpp"
#include "Float.hpp"
#include "String.hpp"
#include "Boolean.hpp"
#include "ObjectConversion.hpp"
#include "xo/reflect/Reflect.hpp"

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
            explicit PrimitiveBase(std::string_view name, Fn impl) : name_{name}, impl_{std::move(impl)} {}

            // inherited from Procedure..

            virtual std::size_t n_args() const override = 0;
            virtual gp<Object> apply_nocheck(gc::IAlloc * mm, const CVector<gp<Object>> & args) override = 0;

        protected:
            /** name for this primitive **/
            std::string_view name_;
            /** implementation **/
            Fn impl_;
        };

        template <typename Fn>
        class Primitive : public PrimitiveBase<Fn> {
        };

        template <typename Ret, typename Arg1, typename Arg2>
        class Primitive<Ret (*)(Arg1, Arg2)> : public PrimitiveBase<Ret (*)(Arg1, Arg2)> {
        public:
            using Super = PrimitiveBase<Ret (*)(Arg1, Arg2)>;
            using function_type = Ret (*)(Arg1, Arg2);
            using TaggedPtr = xo::reflect::TaggedPtr;

        public:
            explicit Primitive(std::string_view name,
                               function_type fn) : PrimitiveBase<Ret (*)(Arg1, Arg2)>{name, fn} {}

            /** see also AdoptPrimitiveExpr::adopt() in xo-interpreter/AdoptPrimitiveExpr.hp **/

            // inherited from Procedure..

            virtual std::size_t n_args() const final override { return 2; }

            virtual gp<Object> apply_nocheck(gc::IAlloc * mm,
                                             const CVector<gp<Object>> & args) final override {
                /* note: args[0] will be this procedure.
                 * actual i'th function argument in args[i+1]
                 */
                Arg1 arg1 = ObjectConversion<Arg1>::from_object(mm, args[1]);
                Arg2 arg2 = ObjectConversion<Arg2>::from_object(mm, args[2]);

                Ret retval = (*Super::impl_)(arg1, arg2);

                return ObjectConversion<Ret>::to_object(mm, retval);
            }

            // inherited from Object..

            virtual TaggedPtr self_tp() const final override {
                using xo::reflect::Reflect;

                return Reflect::make_tp(const_cast<Primitive*>(this));
            }
            virtual void display(std::ostream & os) const final override {
                os << "<primitive" << xtag("name", Super::name_) << ">";
            }
            virtual std::size_t _shallow_size() const final override {
                return sizeof(*this);
            }
            virtual Object *_shallow_copy(gc::IAlloc * mm) const final override {
                Cpof cpof(mm, this);
                return new (cpof) Primitive(*this);
            }
            std::size_t _forward_children(gc::IAlloc *) final override {
                return _shallow_size();
            }
        };

        template <typename Fn>
        gp<Primitive<Fn>>
        make_primitive(gc::IAlloc * mm, std::string_view name, Fn fn) {
            return new (MMPtr(mm)) Primitive<Fn>(name, fn);
        }

    }
}

/* end Primitive.hpp */
