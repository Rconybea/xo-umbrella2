/* file GlobalEnv.cpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#include "xo/indentlog/print/ppdetail_atomic.hpp"
#include "GlobalSymtab.hpp"
#include "Expression.hpp"

namespace xo {
    namespace scm {
        GlobalSymtab::GlobalSymtab() = default;

        bp<Expression>
        GlobalSymtab::require_global(const std::string & vname,
                                  bp<Expression> expr)
        {
            this->global_map_[vname] = expr.get();

            return expr;
        } /*require_global*/

        void
        GlobalSymtab::upsert_local(bp<Variable> target) {
            // in practice: paraphrase of .require_global()

            this->global_map_[target->name()] = target.promote();
        }

        void
        GlobalSymtab::print(std::ostream & os) const {
            os << "<GlobalEnv"
               << xtag("size", global_map_.size())
               << ">";
        }

        std::uint32_t
        GlobalSymtab::pretty_print(const xo::print::ppindentinfo & ppii) const
        {
            using xo::print::ppstate;

            ppstate * pps = ppii.pps();

            if (ppii.upto()) {
                if (!pps->print_upto("<GlobalEnv"))
                    return false;
                if (!pps->print_upto_tag("size", global_map_.size()))
                    return false;
                pps->write(">");

                return true;
            } else {
                pps->write("<GlobalEnv");
                pps->newline_pretty_tag(ppii.ci1(), "size", global_map_.size());
                pps->write(">");

                return false;
            }
        }
    } /*namespace scm*/
} /*namespace xo*/
