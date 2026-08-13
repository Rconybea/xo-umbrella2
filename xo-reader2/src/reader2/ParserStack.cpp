/** @file ParserStack.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "ParserStack.hpp"
#include "SyntaxStateMachine.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/ppsink/concat.hpp>
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/tag_ostream.hpp>

namespace xo {
    /* ppsink xtag for print(ostream&), via the tag_ostream bridge.
     * QUALIFIED at the call sites below: with const char[] arguments ADL
     * still finds a legacy xtag overload, and a using-declaration cannot
     * suppress ADL.
     */

    using xo::print::APrintable;
    using xo::facet::FacetRegistry;
    using xo::facet::typeseq;

    namespace scm {
        ParserStack::ParserStack(DArena::Checkpoint ckp,
                                 obj<ASyntaxStateMachine> ssm,
                                 ParserStack * parent)
            : ckp_{ckp}, ssm_{ssm}, parent_{parent}
        {}

        ParserStack *
        ParserStack::push(ParserStack * stack,
                          DArena::Checkpoint ckp,
                          DArena & mm,
                          obj<ASyntaxStateMachine> ssm)

        {
            //DArena::Checkpoint ckp = mm.checkpoint();  // wrong, must precede allocation of ssm

            void * mem = mm.alloc(typeseq::id<ParserStack>(),
                                  sizeof(ParserStack));

            return new (mem) ParserStack(ckp, ssm, stack);
        }

        ParserStack *
        ParserStack::pop(ParserStack * stack,
                         DArena & mm)
        {
            assert(stack);

            mm.restore(stack->ckp());

            return stack->parent();
        }

        void
        ParserStack::print(std::ostream & os) const
        {
            os << "<ParserStack>";
            os << xo::pp::xtag("ssm", "*placeholder*");
            os << xo::pp::xtag("parent", "*placeholder*");
            os << ">";
        }

        void
        ParserStack::pretty(xo::pp::PpSink & sink) const
        {
            /* force_break: legacy's upto() pass returns false
             * UNCONDITIONALLY ("always use multiple lines"), so this struct
             * never renders flat however wide the margin.  A deliberate
             * layout policy, found by READING the upto() branch above:
             * legacy's own output cannot distinguish "forced" from "did not
             * fit", since it breaks either way.  Dropping force_break IS
             * caught once both protocols are compared, though -- ppsink would
             * render flat where legacy breaks.
             *
             * struct_open rather than pretty_struct: the field count is the
             * stack depth, a runtime value, and pretty_struct has no
             * force_break parameter anyway.
             */
            auto st = sink.struct_open("ParserStack", true /*force_break*/);

            const ParserStack * frame = this;
            std::size_t i_frame = 0;

            while (frame) {
                auto ssm = (FacetRegistry::instance().variant
                            <APrintable, ASyntaxStateMachine> (frame->top()));
                assert(ssm.data());

                /* legacy built this name with snprintf("[%lu]", i_frame). */
                st.field(xo::pp::concat("[", i_frame, "]"), ssm);

                ++i_frame;
                frame = frame->parent_;
            }
        }

        void
        ParserStack::visit_gco_children(VisitReason reason,
                                        obj<AGCObjectVisitor> gc) noexcept
        {

            for (ParserStack * target = this; target; target = target->parent_) {
                // ParserStack::ckp: skip, POD

                if (target->ssm_)
                    target->ssm_.visit_gco_children(reason, gc);
            }
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end ParserStack.cpp */
