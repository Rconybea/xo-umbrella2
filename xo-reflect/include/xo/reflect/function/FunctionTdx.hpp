/** @file FunctionTdx.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/reflect/EstablishTypeDescr.hpp"
#include "xo/reflect/TypeDescrExtra.hpp"

namespace xo {
    namespace reflect {
        /** Additional type-associated information for a function/procedure **/
        class FunctionTdx : public TypeDescrExtra {
        public:
            /** true iff the std::function at the given address is non-empty
             *  -- supplied per std::function type (repr std_function), the
             *  one representation whose emptiness needs the concrete type
             **/
            using IsSetFn = bool (*)(void * object);

        public:
            virtual ~FunctionTdx() = default;

            /** create instance.  Will be invoked exactly once for each reflected function type
             *
             *  @param retval_td.   type description for return value
             *  @param arg_td_v.    type descriptions for arguments, in positional order
             *  @param is_noexcept. true iff function marked noexcept
             **/
            static std::unique_ptr<FunctionTdx> make_function(TypeDescr retval_td,
                                                              std::vector<TypeDescr> arg_td_v,
                                                              bool is_noexcept);
            /** create instance from FunctionTdxInfo
             *  @param fn_info.   function ingredients -- return type, arg types, noexcept
             **/
            static std::unique_ptr<FunctionTdx> make_function(const FunctionTdxInfo & fn_info);

            /** create instance for a std::function type (fn_info.repr_ is
             *  FunctionRepr::std_function), with @p is_set answering
             *  fn_is_set().  See .xo-backlog/xo-reflect/issues/09
             **/
            static std::unique_ptr<FunctionTdx> make_function(const FunctionTdxInfo & fn_info,
                                                              IsSetFn is_set);

            // ----- Inherited from TypeDescrExtra -----

            virtual Metatype metatype() const override { return Metatype::mt_function; }
            virtual uint32_t n_child(void * /*object*/) const override { return 0; }
            virtual uint32_t n_child_fixed() const override { return 0; }
            virtual TaggedPtr child_tp(uint32_t i, void * object) const override;
            virtual TypeDescr fixed_child_td(uint32_t i) const override;
            const std::string & struct_member_name(uint32_t i) const override;

            virtual const FunctionTdxInfo * fn_info() const override { return &info_; }
            virtual TypeDescr fn_retval() const override { return info_.retval_td_; }
            virtual uint32_t n_fn_arg() const override { return info_.arg_td_v_.size(); }
            virtual TypeDescr fn_arg(uint32_t i) const override { return info_.arg_td_v_[i]; }
            virtual bool fn_is_noexcept() const override { return info_.is_noexcept_; }
            virtual FunctionRepr fn_repr() const override { return info_.repr_; }
            virtual bool fn_is_set(void * object) const override;

        private:
            FunctionTdx(const FunctionTdxInfo & fn_info, IsSetFn is_set);

        private:
            /** ingredients in complete function type description **/
            FunctionTdxInfo info_;
            /** repr std_function: answers fn_is_set; nullptr for a pointer **/
            IsSetFn is_set_ = nullptr;
        }; /*FunctionTdx*/
    } /*namespace reflect*/
} /*namespace xo*/


/** end FunctionTdx.hpp **/
