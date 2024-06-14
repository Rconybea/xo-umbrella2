/** @file FunctionTdx.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/reflect/TypeDescrExtra.hpp"
#include "xo/reflect/EstablishTypeDescr.hpp"

namespace xo {
    namespace reflect {
        /** Additional type-associated information for a function/procedure **/
        class FunctionTdx : public TypeDescrExtra {
        public:
            virtual ~FunctionTdx() = default;

            /** create instance.  Will be invoked exactly once for each reflected function type **/
            static std::unique_ptr<FunctionTdx> make_function(TypeDescr retval_td,
                                                              std::vector<TypeDescr> arg_td_v);



            // ----- Inherited from TypeDescrExtra -----

            virtual Metatype metatype() const override { return Metatype::mt_function; }
            virtual uint32_t n_child(void * /*object*/) const override { return 0; }
            virtual TaggedPtr child_tp(uint32_t i, void * object) const override;
            const std::string & struct_member_name(uint32_t i) const override;

            virtual uint32_t n_fn_arg() const override { return arg_td_v_.size(); }
            virtual TypeDescr fn_retval() const override { return retval_td_; }
            virtual TypeDescr fn_arg(uint32_t i) const override { return arg_td_v_[i]; }

        private:
            FunctionTdx(TypeDescr retval_td,
                        std::vector<TypeDescr> arg_td_v);

        private:
            /** function return value **/
            TypeDescr retval_td_;
            /** function arguments,  in positional order **/
            std::vector<TypeDescr> arg_td_v_;
        }; /*FunctionTdx*/
    } /*namespace reflect*/
} /*namespace xo*/


/** end FunctionTdx.hpp **/
