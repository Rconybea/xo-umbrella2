/* @file FunctionTdx.cpp */

#include "function/FunctionTdx.hpp"
#include "TaggedPtr.hpp"
#include "TypeDescr.hpp"

namespace xo {
    namespace reflect {
        /** create instance.  Will be invoked exactly once for each reflected function type **/
        std::unique_ptr<FunctionTdx>
        FunctionTdx::make_function(TypeDescr retval_td,
                                   std::vector<TypeDescr> arg_td_v,
                                   bool is_noexcept)
        {
            return make_function(FunctionTdxInfo(retval_td,
                                                 std::move(arg_td_v),
                                                 is_noexcept));
        }

        std::unique_ptr<FunctionTdx>
        FunctionTdx::make_function(const FunctionTdxInfo & fn_info)
        {
            return std::unique_ptr<FunctionTdx>(new FunctionTdx(fn_info, nullptr));
        }

        std::unique_ptr<FunctionTdx>
        FunctionTdx::make_function(const FunctionTdxInfo & fn_info,
                                   IsSetFn is_set)
        {
            return std::unique_ptr<FunctionTdx>(new FunctionTdx(fn_info, is_set));
        }

        FunctionTdx::FunctionTdx(const FunctionTdxInfo & fn_info, IsSetFn is_set)
            : info_{fn_info}, is_set_{is_set}
        {
            if (!info_.retval_td_) {
                throw std::runtime_error("FunctionTdx::ctor: null return type?");
            }

            /* a std::function's emptiness needs its concrete type: the
             * caller (EstablishTdx<std::function<..>>) supplies it
             */
            if ((info_.repr_ == FunctionRepr::std_function) && !is_set_) {
                throw std::runtime_error("FunctionTdx::ctor: std::function type without is_set?");
            }
        }

        bool
        FunctionTdx::fn_is_set(void * object) const
        {
            switch (info_.repr_) {
            case FunctionRepr::pointer:
                /* a function pointer's storage is a function pointer,
                 * whatever its signature: read it and compare -- for
                 * ingredient-built types too, which have no native type
                 */
                return *reinterpret_cast<void (* const *)()>(object) != nullptr;
            case FunctionRepr::std_function:
                return is_set_(object);
            }

            return false;
        } /*fn_is_set*/

        TaggedPtr
        FunctionTdx::child_tp(uint32_t /*i*/, void * /*object*/) const
        {
            return TaggedPtr::universal_null();
        }

        TypeDescr
        FunctionTdx::fixed_child_td(uint32_t /*i*/) const {
            return nullptr;
        }

        const std::string &
        FunctionTdx::struct_member_name(uint32_t i) const
        {
            return TypeDescrExtra::struct_member_name(i);
        }
    } /*namespace reflect*/
} /*namespace xo*/


/* end FunctionTdx.cpp */
