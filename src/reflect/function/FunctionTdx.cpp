/* @file FunctionTdx.cpp */

#include "function/FunctionTdx.hpp"
#include "TaggedPtr.hpp"

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
            return std::unique_ptr<FunctionTdx>(new FunctionTdx(fn_info));
        }

        FunctionTdx::FunctionTdx(const FunctionTdxInfo & fn_info)
            : info_{fn_info}
        {
            if (!info_.retval_td_) {
                throw std::runtime_error("FunctionTdx::ctor: null return type?");
            }
        }

        TaggedPtr
        FunctionTdx::child_tp(uint32_t /*i*/, void * /*object*/) const
        {
            return TaggedPtr::universal_null();
        }

        const std::string &
        FunctionTdx::struct_member_name(uint32_t i) const
        {
            return TypeDescrExtra::struct_member_name(i);
        }
    } /*namespace reflect*/
} /*namespace xo*/


/* end FunctionTdx.cpp */
