/* file LocalSymtab.cpp
 *
 * author: Roland Conybeare
 */

#include "LocalSymtab.hpp"
#include "pretty_variable.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/PrettyVector.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>
#include <xo/ppsink/pretty_struct.hpp>

namespace xo {
    using xo::pp::xtag;
    using xo::pp::tostr;
    using xo::pp::scope;

    namespace scm {
        rp<LocalSymtab>
        LocalSymtab::make_empty() {
            return new LocalSymtab(std::vector<rp<Variable>>(), nullptr);
        }

        rp<LocalSymtab>
        LocalSymtab::make(const std::vector<rp<Variable>> & argv,
                       const rp<SymbolTable> & parent_env)
        {
            return new LocalSymtab(argv, parent_env);
        }

        rp<LocalSymtab>
        LocalSymtab::make1(const rp<Variable> & arg1,
                        const rp<SymbolTable> & parent_env)
        {
            std::vector<rp<Variable>> argv = { arg1 };

            return make(argv, parent_env);
        }

        LocalSymtab::LocalSymtab(const std::vector<rp<Variable>> & argv,
                           const rp<SymbolTable> & parent_env)
            : origin_{nullptr},
              argv_(argv),
              parent_env_{parent_env}
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG_(c_debug_flag), xtag("this", (void*)this), xtag("argv", argv_));
        }

        binding_path
        LocalSymtab::lookup_local_binding(const std::string & vname) const {
            int j_slot = 0;
            for (const auto & arg : argv_) {
                if (arg->name() == vname)
                    return { 0 /*i_link*/, j_slot };
                ++j_slot;
            }

            return { -2 /*i_link: sentinel*/, 0 };
        } /*lookup_local_binding*/

        binding_path
        LocalSymtab::lookup_binding(const std::string & vname) const {
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
        LocalSymtab::assign_parent(bp<SymbolTable> p) {
            if ((parent_env_.get() != nullptr) && (parent_env_.get() != p.get())) {
                throw std::runtime_error(tostr("LocalSymtab::assign_parent(P2): already have established parent P1",
                                               xtag("P1", parent_env_),
                                               xtag("P2", p)));

                assert(false);
            }

            parent_env_ = p.promote();
        }

        void
        LocalSymtab::upsert_local(bp<Variable> target) {
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
        LocalSymtab::print(std::ostream& os) const {
            os << "<LocalSymtab"
               << xtag("argv", argv_)
               << ">";
        }

        void
        LocalSymtab::pretty(xo::pp::PpSink & sink) const {
            using xo::pp::field;

            /* NB `this` is printed.  The legacy two-pass version disagreed with
             * itself: the fit pass measured `<LocalSymtab :argv ...>` while the
             * print pass emitted `this` as well, so the single-line form could
             * overflow the right margin.  The print branch was the intended
             * one -- xtag("this", (void*)this) is the house idiom for these
             * dumps -- so the fit branch was the bug.  Single-pass makes the
             * two impossible to disagree.
             */
            const void * self = this;

            sink.pretty_struct("LocalSymtab",
                               field("this", self),
                               field("argv", argv_));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end LocalSymtab.cpp */
