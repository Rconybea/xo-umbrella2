/* @file print_eigen.test.cpp
 *
 * Rendering pin for xo::pp::matrix -- the Eigen matrix pretty-printer in
 * xo/kalmanfilter/print_eigen.hpp.
 *
 * Milestone: ostream-containment.  print_eigen.hpp's operator<<(ostream&,
 * matrix<T>) was replaced by Prettifier<matrix<T>> + pretty_matrix(); this
 * file is the coverage that replacement never had.  xo-kalmanfilter is the
 * largest subsystem remaining in that milestone and every KalmanFilter*
 * display() renders matrices through this printer, so a change here reaches
 * all of them at once.
 *
 * Per CONVENTIONS.md every expectation below was READ off the build rather
 * than predicted.  That paid for itself immediately: the first version of
 * this file pinned a leading space that the conversion had introduced
 * without anyone choosing it (see `pp-matrix-no-leading-space`).  Predicting
 * the output would have hidden it, because the space is exactly what you do
 * NOT expect to see.
 *
 * Layout note: pretty_matrix() opens its group with begin_here(), so a
 * matrix that breaks aligns its rows under the opening bracket rather than
 * at the enclosing record's indent.  A FlatSink cannot show that -- see
 * `pp-matrix-emits-break-points`.
 */

#include "xo/kalmanfilter/print_eigen.hpp"
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <Eigen/Dense>
#include <sstream>
#include <string>
#include <algorithm>

namespace xo {
    using xo::pp::FlatSink;
    using xo::pp::matrix;

    namespace ut {
        namespace {
            /** render @p x through a FlatSink and return the text, with
             *  complete()'s trailing newline removed.
             *
             *  FlatSink, not PrettySink: this pins CONTENT.  A FlatSink
             *  renders every split as its space count, so the layout a
             *  PrettySink would produce (breaking one row per line when the
             *  matrix does not fit the margin) is deliberately NOT covered
             *  here -- see `pp-matrix-emits-break-points`.
             **/
            template <typename T>
            std::string flat(const T & x) {
                std::stringstream ss;
                {
                    FlatSink sink(ss.rdbuf());
                    sink.pp(matrix(x));
                    sink.complete();
                }
                std::string s = ss.str();
                if (!s.empty() && s.back() == '\n')
                    s.pop_back();
                return s;
            }

            Eigen::MatrixXd mat(int rows, int cols,
                                std::initializer_list<double> vals) {
                Eigen::MatrixXd m(rows, cols);
                int i = 0;
                for (double v : vals) {
                    m(i / cols, i % cols) = v;
                    ++i;
                }
                return m;
            }
        } /*namespace*/

        TEST_CASE("pp-matrix-shape", "[print_eigen]") {
            /* row separator is ";", column separator is a single space */
            REQUIRE(flat(mat(2, 2, {1, 2, 3, 4})) == "[1 2; 3 4]");
            REQUIRE(flat(mat(1, 3, {1, 2, 3}))    == "[1 2 3]");
            REQUIRE(flat(mat(3, 1, {1, 2, 3}))    == "[1; 2; 3]");
            REQUIRE(flat(mat(1, 1, {7}))          == "[7]");
        }

        TEST_CASE("pp-matrix-empty", "[print_eigen]") {
            /* a 0x0 matrix emits the brackets and nothing else */
            Eigen::MatrixXd m00(0, 0);
            REQUIRE(flat(m00) == "[]");
        }

        TEST_CASE("pp-matrix-vector", "[print_eigen]") {
            /* a VectorXd is Nx1, so it renders as a column: one row each */
            Eigen::VectorXd v(3);
            v << 1.5, -2.25, 0;
            REQUIRE(flat(v) == "[1.5; -2.25; 0]");
        }

        TEST_CASE("pp-matrix-numeric-format", "[print_eigen]") {
            /* elements render through Prettifier<double>, which is %.6g --
             * the same six significant digits ostream defaults to, so
             * retiring operator<<(ostream&, matrix<T>) did NOT change how
             * numbers appear.  Pinned because that equivalence is load-bearing
             * for the ostream-containment milestone's "text preserved" claim.
             */
            REQUIRE(flat(mat(2, 2, {0.1, 1.0 / 3, 1e-8, 123456789.0}))
                    == "[0.1 0.333333; 1e-08 1.23457e+08]");
        }

        TEST_CASE("pp-matrix-no-leading-space", "[print_eigen]") {
            /* RESOLVED 2026-08-23.  This test previously pinned a DEFECT:
             * pretty_matrix() emitted sink.split(1,0) before every row
             * including the first, so every non-empty matrix began "[ ".
             *
             * That was not merely cosmetic.  split(1,0) means "a space, or a
             * break here", so the stray split was offering a break point
             * immediately after "[" -- a place we never want to break.  The
             * leading space was the flat-sink symptom of a layout bug.
             *
             * Fixed by pairing the split with the separator, which also
             * restores the "; " row spacing the retired
             * operator<<(ostream&, matrix<T>) produced.  Kept as a regression
             * pin because the natural way to write this loop reintroduces it.
             */
            REQUIRE(flat(mat(2, 2, {1, 2, 3, 4})).substr(0, 2) == "[1");
            REQUIRE(flat(mat(1, 1, {7})) == "[7]");
        }

        TEST_CASE("pp-matrix-emits-break-points", "[print_eigen]") {
            /* pretty_matrix() wraps its rows in begin()/split()/end() so a
             * PrettySink can break a large matrix one row per line.  A
             * FlatSink renders splits as spaces, so the breaks are invisible
             * here; what this checks is the weaker but still useful property
             * that the row separators are SPLITS rather than hard spaces --
             * i.e. that a wide matrix stays on one line under a flat sink and
             * therefore has somewhere to break under a pretty one.
             *
             * NOT covered: the actual broken layout.  That needs a PrettySink
             * with a narrow margin and belongs with the other PrettySink
             * layout tests rather than here.
             */
            Eigen::MatrixXd wide(4, 4);
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    wide(i, j) = i * 4 + j;

            std::string s = flat(wide);

            REQUIRE(s.find('\n') == std::string::npos);   /* flat: one line */
            REQUIRE(std::count(s.begin(), s.end(), ';') == 3); /* 4 rows -> 3 separators */
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end print_eigen.test.cpp */
