/** @file SetupStringtable2.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "SetupStringtable2.hpp"
#include <xo/stringtable2/String.hpp>
#include <xo/stringtable2/UniqueString.hpp>
#include <xo/stringtable2/string/IReflectable_DString.hpp>
#include <xo/printjson/PrintJson.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <string_view>

namespace xo {
    /* the ppsink logging vocabulary, for use below */
    using xo::pp::scope;
    using xo::pp::xtag;

    using xo::print::APrintable;
    using xo::reflect::AReflectable;
    using xo::mm::ACollector;
    using xo::mm::AGCObject;
    using xo::scm::DString;
    using xo::facet::FacetRegistry;
    using xo::facet::typeseq;
    using xo::facet::impl_for;

    namespace scm {
        namespace {
            /** json printing for DString **/
            class DStringJsonPrinter : public xo::json::JsonPrinter {
            public:
                DStringJsonPrinter(const xo::json::PrintJson * pjson)
                    : xo::json::JsonPrinter(pjson) {}

                virtual void print_json(TaggedPtr tp,
                                        std::ostream * p_os) const override
                {
                    DString * x = this->check_recover_native<DString>(tp, p_os);

                    if (x) {
                        /* through the json printer for string_view, not <<, so
                         * DString shares whatever quoting and escaping every
                         * other string gets.  That escaping is currently
                         * ppsink's rather than JSON's -- an embedded NUL comes
                         * back \x00, not \u0000 -- which is
                         * .xo-backlog/xo-printjson/issues/04, and routing
                         * through the shared printer is what makes it one fix
                         * there rather than a second one here.
                         *
                         * NOT the operator std::string_view(), which stops at
                         * the first null: size_ is the authority on extent.
                         */
                        this->pjson()->print(std::string_view(x->chars(),
                                                              x->size()),
                                             p_os);
                    }
                } /*print_json*/
            }; /*DStringJsonPrinter*/
        }

        void
        SetupStringtable2::provide_json_printers(xo::json::PrintJson * p_pjson)
        {
            assert(p_pjson);

            p_pjson->provide_printer
                (xo::reflect::Reflect::require<DString>(),
                 std::unique_ptr<xo::json::JsonPrinter>
                     (new DStringJsonPrinter(p_pjson)));
        } /*provide_json_printers*/

        bool
        SetupStringtable2::register_facets()
        {
            scope log(XO_DEBUG_(true));

            FacetRegistry::register_impl<AGCObject, DUniqueString>();
            FacetRegistry::register_impl<APrintable, DUniqueString>();

            FacetRegistry::register_impl<AGCObject, DString>();
            FacetRegistry::register_impl<APrintable, DString>();
            /* the runtime half of AReflectable: the FacetImplementation
             * specialization in IReflectable_DString.hpp is compile-time only,
             * and FopTdx's rotation is a FacetRegistry lookup.  Without this
             * line an erased DString compiles, constructs, and throws when
             * something asks it to render.
             */
            FacetRegistry::register_impl<AReflectable, DString>();

            log && log(xtag("DString.tseq", typeseq::id<DString>()));

            return true;
        }

        bool
        SetupStringtable2::register_types(obj<ACollector> gc)
        {
            scope log(XO_DEBUG_(true));

            bool ok = true;

            ok &= gc.install_type(impl_for<AGCObject, DUniqueString>());
            ok &= gc.install_type(impl_for<AGCObject, DString>());

            return ok;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end SetupStringtable2.cpp */
