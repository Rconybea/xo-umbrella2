/* @file Lambda.cpp */

#include "Lambda.hpp"
#include "xo/indentlog/print/vector.hpp"

namespace xo {
    using std::stringstream;

    namespace ast {
        Lambda::Lambda(const std::string & name,
                       const std::vector<std::string> & argv,
                       const ref::rp<Expression> & body)
            : Expression(exprtype::lambda),
              name_{name},
              argv_{argv},
              body_{body}
        {
            stringstream ss;
            ss << "double";
            ss << "(";
            for (std::size_t i = 0; i < argv.size(); ++i) {
                if (i > 0)
                    ss << ",";
                ss << "double";
            }
            ss << ")";

            type_str_ = ss.str();
        } /*ctor*/

        void
        Lambda::display(std::ostream & os) const {
            os << "<Lambda"
               << xtag("name", name_)
               << xtag("argv", argv_)
               << xtag("body", body_)
               << ">";
        } /*display*/
    } /*namespace ast*/
} /*namespace xo*/


/* end Lambda.cpp */
