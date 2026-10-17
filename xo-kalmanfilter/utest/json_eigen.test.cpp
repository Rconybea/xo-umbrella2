/* @file json_eigen.test.cpp
 *
 * Rendering pin for xo-kalmanfilter's json printers -- Eigen vectors and
 * MatrixXd as json arrays (EigenUtil::provide_json_printers).
 *
 * Expectations were READ off the build, before the printers moved from an
 * ostream to a PpSink (.xo-backlog/xo-printjson/issues/01), so the move is
 * checked to be byte-identical.  Since JsonPpSink
 * (.xo-backlog/xo-printjson/issues/11) these lists separate elements with
 * ", " like every other json list, where they used "," alone (RC: rationalize).
 * Elements go through printjson's own printers: non-finite doubles, and the
 * 6-digit precision of .xo-backlog/xo-printjson/issues/10, are theirs.
 */

#include "xo/kalmanfilter/EigenUtil.hpp"
#include <xo/printjson/PrintJson.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/indentlog2/print/TempFlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <catch2/catch.hpp>
#include <Eigen/Dense>
#include <cmath>
#include <limits>
#include <string>

namespace xo {
    using xo::eigen::EigenUtil;
    using xo::json::PrintJson;
    using xo::pp::PpStyle;
    using xo::pp::TempFlatSink;
    using xo::reflect::Reflect;

    namespace ut {
        namespace {
            template <typename T>
            std::string to_json(T const & x) {
                static bool s_once = []() { EigenUtil::reflect_eigen(); return true; }();
                (void)s_once;

                PrintJson pjson;
                EigenUtil::provide_json_printers(&pjson);

                TempFlatSink out(PpStyle::json());
                pjson.print_tp(Reflect::make_tp(const_cast<T *>(&x)), &out.sink());

                return std::string(out.output());
            }
        } /*namespace*/

        TEST_CASE("json-eigen-vectorxd", "[kalmanfilter][json]") {
            Eigen::VectorXd v(4);
            v << 1.0, 2.5, -0.125, std::numeric_limits<double>::quiet_NaN();

            REQUIRE(to_json(v) == "[1, 2.5, -0.125, NaN]");
        }

        TEST_CASE("json-eigen-vectorxb", "[kalmanfilter][json]") {
            Eigen::Array<bool, Eigen::Dynamic, 1> b(3);
            b << true, false, true;

            REQUIRE(to_json(b) == "[true, false, true]");
        }

        TEST_CASE("json-eigen-matrixxd", "[kalmanfilter][json]") {
            Eigen::MatrixXd m(2, 3);
            m << 1, 2, 3,
                 4, 5, std::numeric_limits<double>::infinity();

            REQUIRE(to_json(m) == "[[1, 2, 3], [4, 5, Infinity]]");
        }

        TEST_CASE("json-eigen-empty", "[kalmanfilter][json]") {
            Eigen::VectorXd v(0);
            Eigen::MatrixXd m(0, 0);

            REQUIRE(to_json(v) == "[]");
            REQUIRE(to_json(m) == "[]");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end json_eigen.test.cpp */
