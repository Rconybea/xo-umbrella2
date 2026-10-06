/** @file EnumTdx.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "enum/EnumTdx.hpp"
#include "TaggedPtr.hpp"
#include "TypeDescr.hpp"

namespace xo {
    namespace reflect {
        std::unique_ptr<EnumTdx>
        EnumTdx::make(std::vector<Enumerator> enum_v, LoadFn load, StoreFn store)
        {
            return std::unique_ptr<EnumTdx>(new EnumTdx(std::move(enum_v), load, store));
        } /*make*/

        std::string const *
        EnumTdx::name_of(void const * object) const
        {
            std::int64_t value = load_(object);

            for (Enumerator const & e : enum_v_) {
                if (e.value_ == value)
                    return &e.name_;
            }

            return nullptr;
        } /*name_of*/

        bool
        EnumTdx::assign_from_name(std::string_view name, void * object) const
        {
            for (Enumerator const & e : enum_v_) {
                if (e.name_ == name) {
                    store_(object, e.value_);
                    return true;
                }
            }

            return false;
        } /*assign_from_name*/

        TaggedPtr
        EnumTdx::child_tp(uint32_t /*i*/, void * /*object*/) const {
            return TaggedPtr::universal_null();
        } /*child_tp*/

        TypeDescr
        EnumTdx::fixed_child_td(uint32_t /*i*/) const {
            return nullptr;
        } /*fixed_child_td*/

        std::string const &
        EnumTdx::struct_member_name(uint32_t i) const {
            return TypeDescrExtra::struct_member_name(i);
        } /*struct_member_name*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end EnumTdx.cpp */
