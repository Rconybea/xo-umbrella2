/* @file Refcounted_ostream.test.cpp
 *
 *  Pin that including Refcounted_ostream.hpp actually makes
 *
 *      os << some_rp
 *
 *  render the pointee.  This needs a test rather than a compile check because
 *  intrusive_ptr<T>::operator bool() is non-explicit (Refcounted.hpp:134), so
 *  when the inserter is not visible the expression still compiles -- and
 *  quietly prints "1".  A unit test is the only thing that tells those apart.
 **/

#include "Refcounted_ostream.hpp"
#include <catch2/catch.hpp>
#include <sstream>

namespace xo {
    namespace ut {
        namespace {
            /* deliberately in an unnamed namespace inside xo::ut, i.e. NOT in
             * xo::ref and NOT in xo::pp -- a consumer's namespace, which is
             * what decides ADL for `os << rp<Pointee>`.
             */
            class Pointee : public ref::Refcount {
            public:
                Pointee(int x) : x_{x} {}
                int x() const { return x_; }
            private:
                int x_ = 0;
            };

            inline std::ostream &
            operator<<(std::ostream & os, const Pointee & x) {
                os << "Pointee(" << x.x() << ")";
                return os;
            }
        }

        TEST_CASE("refcounted-ostream-rp", "[refcnt]") {
            rp<Pointee> p(new Pointee(17));

            std::stringstream ss;
            ss << p;

            REQUIRE(ss.str() == "Pointee(17)");
        }

        TEST_CASE("refcounted-ostream-rp-null", "[refcnt]") {
            rp<Pointee> p;

            std::stringstream ss;
            ss << p;

            /* type_name<> spelling varies; what matters is that it is not "0" */
            REQUIRE(ss.str().starts_with("<nullptr "));
        }

        TEST_CASE("refcounted-ostream-bp", "[refcnt]") {
            rp<Pointee> p(new Pointee(23));
            bp<Pointee> b = p.borrow();

            std::stringstream ss;
            ss << b;

            REQUIRE(ss.str() == "Pointee(23)");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end Refcounted_ostream.test.cpp */
