/* file formal_arg.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "TypeDescr.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/Prettifier.hpp>
#include <xo/ppsink/tag.hpp>

namespace xo {
    namespace scm {
        /** @class formal_arg
         *  @brief description of formal parameter in an argument list
         *
         *  Terminated by an argument separator ',' or rightparen ')'
         **/
        class formal_arg {
        public:
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            formal_arg() = default;
            formal_arg(const std::string & n, TypeDescr td) : name_{n}, td_{td} {}

            const std::string & name() const { return name_; }
            TypeDescr td() const { return td_; }

            void assign_name(const std::string & x) { name_ = x; }
            void assign_td(TypeDescr x) { td_ = x; }

            // keeping until we're certain formal_arg::pretty() is implemented
#ifdef OBSOLETE
            void print(std::ostream & os) const {
                using xo::pp::xtag;

                os << "<formal_arg";
                if (!name_.empty())
                    os << xtag("name", name_);
                if (td_)
                    os << xtag("td", td_);
                os << ">";
            }
#endif

            void pretty(PpSink & sink) const;

        private:
            /** formal parameter name **/
            std::string name_;
            /** type description for variable @p name **/
            TypeDescr td_ = nullptr;
        };

    } /*namespace scm*/

    namespace pp {
        XO_PRETTIFIER_DECLARE(xo::scm::formal_arg);
    }
} /*namespace xo*/


/* end formal_arg.hpp */
