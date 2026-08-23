/* @file kalmanfilter_pretty.test.cpp
 *
 * Rendering pin for the xo-kalmanfilter value types, after their
 * display(std::ostream&) printers were replaced by pretty(PpSink&) +
 * Prettifier<> (ostream-containment milestone).
 *
 * WHY THIS EXISTS, AND WHAT IT IS NOT
 *
 * The xo-reflect conversion was guarded by ostream_baseline.test.cpp, which
 * held the OLD and NEW printers side by side and asserted their text agreed --
 * "THE claim that makes deleting display() safe".  That is not possible here,
 * and the reason is worth recording:
 *
 *   xo-kalmanfilter's retired display() bodies are behind #ifdef OBSOLETE, and
 *   OBSOLETE is a TREE-WIDE marker.  Defining it to expose display() also
 *   un-guards unrelated code in other subsystems -- xo-reflect's
 *   StructMember.hpp:85 member_tp() override, for one -- which then fails to
 *   compile.  So the two implementations cannot be built together, and no
 *   automated comparison between them is available.
 *
 * Consequence, stated rather than left implicit: these expectations pin what
 * pretty() produces TODAY.  They were read off the build, not predicted, and
 * they were eyeballed against the display() bodies field-by-field -- but
 * nothing here proves the conversion was text-preserving, because nothing can.
 * Treat a failure as "the rendering changed", not as "the rendering regressed".
 *
 * Covers the types with public constructors.  KalmanFilterState,
 * KalmanFilterStateExt, KalmanFilterStep, KalmanFilterSpec and KalmanFilter
 * are built through factories/private ctors and are exercised only indirectly
 * by KalmanFilter.test.cpp; they are NOT pinned here.
 */

#include "xo/kalmanfilter/KalmanFilterTransition.hpp"
#include "xo/kalmanfilter/KalmanFilterObservable.hpp"
#include "xo/kalmanfilter/print_eigen.hpp"
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <Eigen/Dense>
#include <sstream>
#include <string>

namespace xo {
    using xo::kalman::KalmanFilterTransition;
    using xo::kalman::KalmanFilterObservable;
    using xo::pp::FlatSink;

    namespace ut {
        namespace {
            /** drop ANSI SGR sequences.  The default style IS coloured here
             *  (field names arrive wrapped in struct_tag_color), so a raw
             *  comparison would pin the palette as well as the text.  Colour
             *  is a separate concern and deliberately not covered.
             **/
            std::string strip_ansi(const std::string & s) {
                std::string out;
                for (std::size_t i = 0; i < s.size(); ++i) {
                    if (s[i] == '\033') {
                        while (i < s.size() && s[i] != 'm')
                            ++i;
                        continue;
                    }
                    out.push_back(s[i]);
                }
                return out;
            }

            template <typename T>
            std::string flat(const T & x) {
                std::stringstream ss;
                {
                    FlatSink sink(ss.rdbuf());
                    sink.pp(x);
                    sink.complete();
                }
                std::string s = ss.str();
                if (!s.empty() && s.back() == '\n')
                    s.pop_back();
                return strip_ansi(s);
            }

            KalmanFilterTransition make_transition() {
                Eigen::MatrixXd F(2, 2); F << 1, 0, 0, 1;
                Eigen::MatrixXd Q(2, 2); Q << 0.5, 0, 0, 0.25;
                return KalmanFilterTransition(F, Q);
            }

            KalmanFilterObservable make_observable() {
                Eigen::VectorXi keep(1); keep << 0;
                Eigen::MatrixXd H(1, 2); H << 1, 0;
                Eigen::MatrixXd R(1, 1); R << 0.25;
                return KalmanFilterObservable(keep, H, R);
            }
        } /*namespace*/

        TEST_CASE("kf-pretty-transition", "[kalmanfilter-pretty]") {
            REQUIRE(flat(make_transition())
                    == "<KalmanFilterTransition :F [1 0; 0 1] :Q [0.5 0; 0 0.25]>");
        }

        TEST_CASE("kf-pretty-observable", "[kalmanfilter-pretty]") {
            /* NB :keep is a member but is NOT rendered -- the retired display()
             * did not print it either, so this is preserved behaviour rather
             * than an omission introduced by the conversion.
             */
            REQUIRE(flat(make_observable())
                    == "<KalmanFilterObservable :H [1 0] :R [0.25]>");
        }

        TEST_CASE("kf-pretty-display-string-agrees", "[kalmanfilter-pretty]") {
            /* display_string() was `std::stringstream ss; ss << *this;` (or
             * `this->display(ss)`) and is now xo::pp::tostr(*this).  This pins
             * that the two entry points -- sink.pp(x) and x.display_string() --
             * still produce the same text, which is what callers of
             * display_string() depend on.
             */
            KalmanFilterTransition tr = make_transition();

            REQUIRE(strip_ansi(tr.display_string()) == flat(tr));
        }

        TEST_CASE("kf-pretty-matrices-nest", "[kalmanfilter-pretty]") {
            /* The matrix fields render through Prettifier<xo::pp::matrix<T>>
             * (print_eigen.hpp), so a change there reaches every one of these
             * types.  print_eigen.test.cpp pins that printer directly; this
             * checks the nesting actually happens -- i.e. that a matrix field
             * arrives as "[...]" and not as an address or an opaque token.
             */
            std::string s = flat(make_transition());

            REQUIRE(s.find(":F [1 0; 0 1]") != std::string::npos);
            REQUIRE(s.find(":Q [0.5 0; 0 0.25]") != std::string::npos);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end kalmanfilter_pretty.test.cpp */
