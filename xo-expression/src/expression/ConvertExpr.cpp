/* file ConvertExpr.cpp
 *
 * author: Roland Conybeare
 */

#include "ConvertExpr.hpp"
#include <xo/ppsink/tag_ostream.hpp>
#include <xo/ppsink/quoted.hpp>
#include <xo/ppsink/pretty_struct.hpp>

namespace xo {
    using xo::pp::field;
    using xo::pp::xtag;
    namespace scm {
        rp<ConvertExpr>
        ConvertExpr::make(TypeDescr dest_type,
                          rp<Expression> arg)
        {
            return new ConvertExpr(dest_type,
                                   std::move(arg));
        }

        std::set<std::string>
        ConvertExpr::get_free_variables() const {
            if (this->arg_)
                return this->arg_->get_free_variables();
            else
                return std::set<std::string>();
        }

        void
        ConvertExpr::pretty(xo::pp::PpSink & sink) const {
            sink.pretty_struct("Convert",
                               field("dest_type", xo::pp::quot(this->valuetype()->short_name())),
                               field("arg", arg_));
        }

        // ----- ConvertExprAccess -----

        rp<ConvertExprAccess>
        ConvertExprAccess::make(TypeDescr dest_type,
                                rp<Expression> arg)
        {
            return new ConvertExprAccess(dest_type,
                                         std::move(arg));
        }

        rp<ConvertExprAccess>
        ConvertExprAccess::make_empty() {
            return new ConvertExprAccess(nullptr /*dest_type*/,
                                         nullptr /*arg*/);
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end ConvertExpr.cpp */
