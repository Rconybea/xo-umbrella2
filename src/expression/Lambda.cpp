/* @file Lambda.cpp */

#include "Lambda.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include "xo/reflect/function/FunctionTdx.hpp"
#include "xo/indentlog/print/vector.hpp"

namespace xo {
    using xo::reflect::TypeDescrBase;
    using xo::reflect::FunctionTdxInfo;
    using xo::ref::rp;
    using std::stringstream;

    namespace ast {
        rp<Lambda>
        Lambda::make(const std::string & name,
                     const std::vector<rp<Variable>> & argv,
                     const ref::rp<Expression> & body)
        {
            using xo::reflect::FunctionTdx;

            /** assemble function type.
             *
             *  NOTE: need this to be unique!
             **/

            std::vector<TypeDescr> arg_td_v;
            arg_td_v.reserve(argv.size());

            for (const auto & arg : argv) {
                arg_td_v.push_back(arg->valuetype());
            }

            auto function_info
                = FunctionTdxInfo(body->valuetype(),
                                  arg_td_v,
                                  false /*!is_noexcept*/);

            TypeDescr lambda_td
                = TypeDescrBase::require_by_fn_info(function_info);

            return new Lambda(name,
                              lambda_td,
                              LocalEnv::make(argv),
                              body);
        } /*make*/

        Lambda::Lambda(const std::string & name,
                       TypeDescr lambda_type,
                       const rp<LocalEnv> & local_env,
                       const ref::rp<Expression> & body)
            : FunctionInterface(exprtype::lambda, lambda_type),
              name_{name},
              body_{body},
              local_env_{local_env}
        {
            stringstream ss;
            ss << "double";
            ss << "(";
            for (std::size_t i = 0, n = this->n_arg(); i < n; ++i) {
                if (i > 0)
                    ss << ",";
                ss << "double";
            }
            ss << ")";

            type_str_ = ss.str();

            body_->attach_envs(local_env_);
        } /*ctor*/

        void
        Lambda::display(std::ostream & os) const {
            os << "<Lambda"
               << xtag("name", name_)
               << xtag("argv", local_env_->argv())
               << xtag("body", body_)
               << ">";
        } /*display*/
    } /*namespace ast*/
} /*namespace xo*/


/* end Lambda.cpp */
