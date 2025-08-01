/* file LocalEnv.cpp
 *
 * author: Roland Conybeare
 */

#include "LocalEnv.hpp"
#include "pretty_variable.hpp"
#include "xo/indentlog/print/pretty_vector.hpp"
#include "xo/indentlog/print/vector.hpp"


namespace xo {
    namespace scm {
        rp<LocalEnv>
        LocalEnv::make_empty() {
            return new LocalEnv(std::vector<rp<Variable>>(), nullptr);
        }

        rp<LocalEnv>
        LocalEnv::make(const std::vector<rp<Variable>> & argv,
                       const rp<Environment> & parent_env)
        {
            return new LocalEnv(argv, parent_env);
        }

        rp<LocalEnv>
        LocalEnv::make1(const rp<Variable> & arg1,
                        const rp<Environment> & parent_env)
        {
            std::vector<rp<Variable>> argv = { arg1 };

            return make(argv, parent_env);
        }

        LocalEnv::LocalEnv(const std::vector<rp<Variable>> & argv,
                           const rp<Environment> & parent_env)
            : origin_{nullptr},
              argv_(argv),
              parent_env_{parent_env}
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag), xtag("this", (void*)this), xtag("argv", argv_));
        }

        binding_path
        LocalEnv::lookup_local_binding(const std::string & vname) const {
            int j_slot = 0;
            for (const auto & arg : argv_) {
                if (arg->name() == vname)
                    return { 0 /*i_link*/, j_slot };
                ++j_slot;
            }

            return { -2 /*i_link: sentinel*/, 0 };
        } /*lookup_local_binding*/

        binding_path
        LocalEnv::lookup_binding(const std::string & vname) const {
            {
                auto local = this->lookup_local_binding(vname);
                if (local.i_link_ == 0)
                    return local;
            }

            auto free = parent_env_->lookup_binding(vname);

            if (free.i_link_ == -1)
                return free;
            else
                return { free.i_link_ + 1, free.j_slot_ };
        } /*lookup_binding*/

        void
        LocalEnv::assign_parent(bp<Environment> p) {
            if ((parent_env_.get() != nullptr) && (parent_env_.get() != p.get())) {
                throw std::runtime_error(tostr("LocalEnv::assign_parent(P2): already have established parent P1",
                                               xtag("P1", parent_env_),
                                               xtag("P2", p)));

                assert(false);
            }

            parent_env_ = p.promote();
        }

        void
        LocalEnv::upsert_local(bp<Variable> target) {
            for (auto & var : this->argv_)  {
                if (var->name() == target->name()) {
                    /* replace existing variable.  This may change its type */
                    var = target.promote();
                    return;
                }
            }

            /* control here: target not already present in this frame -> append */

            this->argv_.push_back(target.promote());
        }

        void
        LocalEnv::print(std::ostream& os) const {
            os << "<LocalEnv"
               << xtag("argv", argv_)
               << ">";
        }

        std::uint32_t
        LocalEnv::pretty_print(const xo::print::ppindentinfo & ppii) const {
            using xo::print::ppstate;

            ppstate * pps = ppii.pps();

            if (ppii.upto()) {
                if (!pps->print_upto("<LocalEnv"))
                    return false;
                if (!pps->print_upto_tag("argv", argv_))
                    return false;
                pps->write(">");

                return true;
            } else {
                pps->write("<LocalEnv");
                pps->newline_pretty_tag(ppii.ci1(), "this", (void*)this);
                pps->newline_pretty_tag(ppii.ci1(), "argv", argv_);
                pps->write(">");

                return false;
            }
        }

    } /*namespace scm*/
} /*namespace xo*/


/* end LocalEnv.cpp */
