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
            return std::unique_ptr<FunctionTdx>(new FunctionTdx(retval_td,
                                                                is_noexcept,
                                                                std::move(arg_td_v)));
        }

        FunctionTdx::FunctionTdx(TypeDescr retval_td,
                                 bool is_noexcept,
                                 std::vector<TypeDescr> arg_td_v)
            : info_{retval_td, std::move(arg_td_v), is_noexcept}
        {
            if (!retval_td) {
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
