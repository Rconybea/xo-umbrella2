/* file GlobalEnv.cpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#include "GlobalSymtab.hpp"
#include "Expression.hpp"
#include <xo/ppsink/tag_ostream.hpp>
#include <xo/ppsink/pretty_struct.hpp>

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
        GlobalSymtab::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            const std::size_t size = global_map_.size();

            sink.pretty_struct("GlobalEnv", field("size", size));
        }
    } /*namespace scm*/
} /*namespace xo*/
