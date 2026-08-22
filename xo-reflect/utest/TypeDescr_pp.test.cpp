/* file TypeDescr_pp.test.cpp
 *
 * exercise TypeDescrBase::pretty() + Prettifier<TypeDescr> --
 * xo-reflect's ppsink-native structured printing.
 *
 * Before the ppsink migration this path had no coverage at all: pretty() was
 * reachable only through legacy indentlog's ppdetail<>, which no test drove.
 * That is how a printer regression would previously have gone unnoticed.
 */

#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;
    using xo::pp::FlatSink;
    using std::stringstream;

    namespace ut {
        namespace {
            /** flat rendering of @p td through the ppsink Prettifier **/
            std::string pretty_of(TypeDescr td) {
                stringstream ss;
                FlatSink sink(ss.rdbuf());
                sink.pp(td);
                sink.complete();
                return ss.str();
            }
        } /*namespace*/

        TEST_CASE("TypeDescr_pp-atomic", "[TypeDescr_pp]") {
            /* NB the explicit TypeDescr (const TypeDescrBase *): Reflect::require
             * returns TypeDescrW (non-const), which Prettifier<TypeDescr> does
             * not match -- such a value falls back to operator<< instead.  Legacy
             * ppdetail<> had no TypeDescrW specialization either, so this
             * mirrors the old behaviour rather than changing it.
             */
            TypeDescr td = Reflect::require<int>();

            std::string s = pretty_of(td);

            INFO(s);
            REQUIRE(s.substr(0, 11) == "<TypeDescr ");
            REQUIRE(s.find(":id ") != std::string::npos);
            REQUIRE(s.find(":canonical_name int") != std::string::npos);
            REQUIRE(s.find(":complete ") != std::string::npos);
            REQUIRE(s.find(":metatype ") != std::string::npos);
            /* trailing '>' then FlatSink's newline from complete() */
            REQUIRE(s.find('>') != std::string::npos);
        }

        TEST_CASE("TypeDescr_pp-distinct-types", "[TypeDescr_pp]") {
            TypeDescr td_i = Reflect::require<int>();
            TypeDescr td_d = Reflect::require<double>();

            std::string si = pretty_of(td_i);
            std::string sd = pretty_of(td_d);

            REQUIRE(si.find(":canonical_name int") != std::string::npos);
            REQUIRE(sd.find(":canonical_name double") != std::string::npos);
            REQUIRE(si != sd);
        }

        TEST_CASE("TypeDescr_pp-null-prints-nullptr", "[TypeDescr_pp]") {
            /* CHANGED 2026-08-22: this pinned "prints nothing", preserving
             * legacy ppdetail<TypeDescr> (`td ? td->pretty(..) : true`).  The
             * ostream conversion retired that legacy along with
             * operator<<(ostream&, const TypeDescrBase*), which DID print
             * "<nullptr>" -- so Prettifier<TypeDescr> adopted "<nullptr>"
             * rather than inheriting silence by default.  See the CHANGED note
             * on that specialization in TypeDescr.hpp.
             */
            TypeDescr td = nullptr;

            stringstream ss;
            FlatSink sink(ss.rdbuf());
            sink.pp(td);
            sink.complete();

            REQUIRE(ss.str() == "<nullptr>\n");   /* + complete()'s newline */
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end TypeDescr_pp.test.cpp */
