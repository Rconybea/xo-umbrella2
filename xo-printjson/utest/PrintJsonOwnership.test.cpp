/** @file PrintJsonOwnership.test.cpp
 *
 *  placement by ownership: owning and shared edges place an object,
 *  borrowed edges only name it -- .xo-backlog/xo-printjson/issues/08.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/printjson/PrintJson.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/SelfTagging.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <catch2/catch.hpp>
#include <memory>
#include <sstream>
#include <string>

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::Reflect;
    using xo::reflect::SelfTagging;
    using xo::reflect::StructReflector;
    using xo::reflect::TaggedRcptr;

    namespace ut {
        namespace {
            struct Leaf {
                int v_ = 0;
            };

            /** self-tagging: reaching it through child_tp() would call
             *  self_tp(), reading the object
             **/
            struct Tagged : public SelfTagging {
                TaggedRcptr self_tp() override { return Reflect::make_rctp(this); }

                int w_ = 0;
            };

            /** borrows a Tagged: never placed here **/
            struct BorrowsTagged {
                int n_ = 0;
                Tagged * t_ = nullptr;
            };

            /** a Leaf by value, and a borrowed pointer to it **/
            struct OwnsAndBorrows {
                int n_ = 0;
                Leaf leaf_;
                Leaf * raw_ = nullptr;
            };

            /** a borrowed pointer to its own first member: same address as
             *  the whole, a different type
             **/
            struct FirstMemberRef {
                Leaf leaf_;
                Leaf * raw_ = nullptr;
            };

            struct Base {
                int b_ = 0;
            };

            struct Derived : public Base {
                int d_ = 0;
            };

            /** a Derived owned, and borrowed as its Base **/
            struct OwnsDerived {
                std::unique_ptr<Derived> d_;
                Base * b_ = nullptr;
            };

            struct Counted : public ref::Refcount {
                int c_ = 0;
            };

            /** overrides: a raw pointer that places, an rp that does not **/
            struct Overrides {
                Leaf * raw_owned_ = nullptr;
                rp<Counted> shared_borrowed_;
            };

            void reflect_types() {
                static bool s_once = []() {
                    {
                        StructReflector<Leaf> sr;
                        sr.reflect_member("v", &Leaf::v_);
                    }
                    {
                        StructReflector<Tagged> sr;
                        sr.reflect_member("w", &Tagged::w_);
                    }
                    {
                        StructReflector<BorrowsTagged> sr;
                        sr.reflect_member("n", &BorrowsTagged::n_);
                        sr.reflect_member("t", &BorrowsTagged::t_);
                    }
                    {
                        StructReflector<OwnsAndBorrows> sr;
                        sr.reflect_member("n", &OwnsAndBorrows::n_);
                        sr.reflect_member("leaf", &OwnsAndBorrows::leaf_);
                        sr.reflect_member("raw", &OwnsAndBorrows::raw_);
                    }
                    {
                        StructReflector<FirstMemberRef> sr;
                        sr.reflect_member("leaf", &FirstMemberRef::leaf_);
                        sr.reflect_member("raw", &FirstMemberRef::raw_);
                    }
                    {
                        StructReflector<Base> sr;
                        sr.reflect_member("b", &Base::b_);
                    }
                    {
                        StructReflector<Derived> sr;
                        sr.adopt_parent<Base>();
                        sr.reflect_member("d", &Derived::d_);
                    }
                    {
                        StructReflector<OwnsDerived> sr;
                        sr.reflect_member("d", &OwnsDerived::d_);
                        sr.reflect_member("b", &OwnsDerived::b_);
                    }
                    {
                        StructReflector<Counted> sr;
                        sr.reflect_member("c", &Counted::c_);
                    }
                    {
                        StructReflector<Overrides> sr;
                        sr.reflect_member("raw_owned", &Overrides::raw_owned_).owning();
                        sr.reflect_member("shared_borrowed", &Overrides::shared_borrowed_).borrowed();
                    }
                    return true;
                }();
                (void)s_once;
            }

            template <typename T>
            std::string print(T const & x) {
                PrintJson pjson;
                std::stringstream ss;
                pjson.print(x, &ss);
                return ss.str();
            }

            bool contains(std::string const & s, std::string const & pat) {
                return s.find(pat) != std::string::npos;
            }
        } /*namespace*/

        TEST_CASE("borrowed-pointer-never-read", "[printjson][ownership]") {
            /* a borrowed pointer to no object at all: reading it -- even
             * asking a self-tagging pointee its type -- would crash
             */
            reflect_types();

            BorrowsTagged x;
            x.n_ = 1;
            x.t_ = reinterpret_cast<Tagged *>(0x10);

            std::string s = print(x);

            INFO(s);
            REQUIRE(contains(s, "\"_value_\": {\"_ref_\": 2}"));
            /* never placed: reported, with the pointee type and address */
            REQUIRE(contains(s, "\"_unplaced_\": [{\"_ref_\": 2, \"_type_\": \"Tagged\", \"_address_\": 16}]"));
        }

        TEST_CASE("borrowed-null-pointer", "[printjson][ownership]") {
            reflect_types();

            BorrowsTagged x;

            std::string s = print(x);

            INFO(s);
            REQUIRE(contains(s, "\"_value_\": null"));
            REQUIRE(!contains(s, "_unplaced_"));
        }

        TEST_CASE("borrowed-ref-to-a-placed-object", "[printjson][ownership]") {
            /* leaf_ placed by value (object 2); raw_ refers to it: no trailer */
            reflect_types();

            OwnsAndBorrows x;
            x.raw_ = &x.leaf_;

            std::string s = print(x);

            INFO(s);
            REQUIRE(contains(s, "\"_id_\": 2"));
            REQUIRE(contains(s, "\"_value_\": {\"_ref_\": 2}"));
            REQUIRE(!contains(s, "_unplaced_"));
        }

        TEST_CASE("borrowed-ref-to-a-first-member", "[printjson][ownership]") {
            /* raw_ points at leaf_, at the FirstMemberRef's own address:
             * the object placed there is the whole, not a Leaf, so the ref
             * is reported
             */
            reflect_types();

            FirstMemberRef x;
            x.raw_ = &x.leaf_;

            REQUIRE(static_cast<void *>(&x) == static_cast<void *>(&x.leaf_));

            std::string s = print(x);

            INFO(s);
            REQUIRE(contains(s, "\"_unplaced_\": [{\"_ref_\": 1, \"_type_\": \"Leaf\""));
        }

        TEST_CASE("borrowed-base-pointer-to-a-placed-derived", "[printjson][ownership]") {
            /* d_ owns a Derived (placed, object 2); b_ borrows it as its
             * Base.  Derived declares Base its parent: placed for the ref
             */
            reflect_types();

            OwnsDerived x;
            x.d_ = std::make_unique<Derived>();
            x.b_ = x.d_.get();

            std::string s = print(x);

            INFO(s);
            REQUIRE(contains(s, "\"_value_\": {\"_ref_\": 2}"));
            REQUIRE(!contains(s, "_unplaced_"));
        }

        TEST_CASE("ownership-overrides", "[printjson][ownership]") {
            /* raw_owned_ (owning): the Leaf in full.  shared_borrowed_
             * (borrowed): only a ref, so its Counted is unplaced
             */
            reflect_types();

            Leaf leaf;
            leaf.v_ = 7;

            Overrides x;
            x.raw_owned_ = &leaf;
            x.shared_borrowed_ = new Counted();

            std::string s = print(x);

            INFO(s);
            REQUIRE(contains(s, "\"_name_\": \"Leaf\""));
            REQUIRE(!contains(s, "\"_name_\": \"Counted\""));
            REQUIRE(contains(s, "\"_unplaced_\": [{\"_ref_\": 3, \"_type_\": \"Counted\""));
        }

        TEST_CASE("top-level-pointer-places-its-target", "[printjson][ownership]") {
            /* a raw pointer handed to an entry point: the caller vouches for
             * it, so its Leaf prints in full, as the top-level object
             */
            reflect_types();

            Leaf leaf;
            leaf.v_ = 5;
            Leaf * p = &leaf;

            std::string s = print(p);

            INFO(s);
            REQUIRE(contains(s, "\"_name_\": \"Leaf\""));
            REQUIRE(contains(s, "\"_value_\": 5"));
            REQUIRE(!contains(s, "_unplaced_"));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end PrintJsonOwnership.test.cpp */
