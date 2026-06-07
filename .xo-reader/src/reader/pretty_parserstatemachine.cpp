/* file pretty_parserstatemachine.cpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#include "pretty_parserstatemachine.hpp"
#include "pretty_exprstatestack.hpp"
#include "pretty_envframestack.hpp"
#include "exprstatestack.hpp"

namespace xo {
    namespace print {
        bool
        ppdetail<xo::scm::parserstatemachine>::print_pretty(const ppindentinfo & ppii, const xo::scm::parserstatemachine & x)
        {
            ppstate * pps = ppii.pps();

            if (ppii.upto()) {
                if (!pps->print_upto("<psm"))
                    return false;

                if (!pps->print_upto_tag("stack", &x.xs_stack_))
                    return false;

                if (!pps->print_upto_tag("env_stack", &x.env_stack_))
                    return false;

                return pps->print_upto(">");
            } else {
                pps->write("<psm");
                pps->newline_pretty_tag(ppii.ci1(), "stack", &x.xs_stack_);
                pps->newline_pretty_tag(ppii.ci1(), "env_stack", &x.env_stack_);
                pps->write(">");

                return false;
            }
        }

        bool
        ppdetail<xo::scm::parserstatemachine *>::print_pretty(const ppindentinfo & ppii, const xo::scm::parserstatemachine * x)
        {
            if (x) {
                return ppdetail<xo::scm::parserstatemachine>::print_pretty(ppii, *x);
            } else {
                if (ppii.upto()) {
                    return ppii.pps()->print_upto("<nullptr>");
                } else {
                    ppii.pps()->write("<nullptr>");
                    return false;
                }
            }
        }
    } /*namespace print*/
} /*namespace xo*/
