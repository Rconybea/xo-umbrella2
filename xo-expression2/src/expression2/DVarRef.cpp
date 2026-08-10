/** @file DVarRef.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "DVarRef.hpp"
#include <xo/ppsink/pretty_struct.hpp>   /* sink.pretty_struct(..), field(..) */
#include <string_view>

namespace xo {
    using xo::mm::AGCObject;
    using xo::reflect::TypeDescr;

    namespace scm {

        DVarRef::DVarRef(DVariable * vardef,
                         Binding path)
                : vardef_{vardef},
                  path_{path}
        {}

        DVarRef *
        DVarRef::make(obj<AAllocator> mm,
                      DVariable * vardef,
                      int32_t link)
        {
            assert(vardef);

            void * mem = mm.alloc_for<DVarRef>();

            return new (mem) DVarRef(vardef,
                                     Binding::relative(link,
                                                       vardef->path()));
        }

        const DUniqueString *
        DVarRef::name() const {
            return vardef_->name();
        }

        TypeRef
        DVarRef::typeref() const noexcept {
            assert(vardef_);

            return vardef_->typeref();
        }

        TypeDescr
        DVarRef::valuetype() const noexcept
        {
            return this->typeref().td();
        }

        void
        DVarRef::assign_valuetype(TypeDescr td) noexcept
        {
            assert(vardef_);
            vardef_->assign_valuetype(td);
        }

        // gcobject facet

        DVarRef *
        DVarRef::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DVarRef::visit_gco_children(VisitReason reason,
                                    obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &vardef_);
            //auto iface = xo::facet::impl_for<AGCObject,DVariable>();
            //gc.forward_inplace(&iface, (void **)vardef_.data_);

            // TODO: concept to indicate that no gc pointers in Binding
        }

        // printable facet

        bool
        DVarRef::pretty_deprecated(const ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct
                       (ppii,
                        "DVarRef",
                        refrtag("name", std::string_view(*(this->name()))),
                        refrtag("path", this->path_));
        }

        void
        DVarRef::pretty(xo::pp::PpSink & sink) const
        {
            /* named locals: field() captures BY REFERENCE (pretty_struct.hpp) */
            const DUniqueString * sym = this->name();

            /* legacy dereferenced name() unguarded; a DVariable's name_ has no
             * non-null invariant (DVariable::pretty guards it), so that is a
             * latent null deref rather than a rendering decision.  Guarded here
             * to match the sibling printer; output-identical wherever legacy is
             * defined.  No quot(), also matching legacy -- DVariable quotes its
             * :name and DVarRef does not, and unifying them WOULD be an
             * output-visible change.
             */
            auto name = (sym
                         ? std::string_view(*sym)
                         : std::string_view(""));

            /* Binding has no Prettifier<> and no ppdetail<> -- only an
             * operator<< (Binding.hpp:58).  It therefore takes ppsink's leaf
             * FALLBACK, which is the thing this printer is here to pin.
             */
            sink.pretty_struct("DVarRef",
                               xo::pp::field("name", name),
                               xo::pp::field("path", path_));
        }

    }
} /*namespace xo*/

/* end DVarRef.cpp */
