/** @file WrapperReflector.test.cpp
 *
 *  transparent wrappers -- see .xo-backlog/xo-reflect/issues/04.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/reflect/WrapperReflector.hpp"
#include "xo/reflect/Reflect.hpp"
#include <catch2/catch.hpp>
#include <cstdint>

namespace xo {
    using xo::reflect::Metatype;
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;
    using xo::reflect::WrapperReflector;
    using xo::reflect::WrapperTdx;

    namespace ut {
        namespace {
            /* a wrapper with a private value: reflected through a public
             * member-pointer accessor, as fn::CallbackIdImpl does
             */
            class Ticket {
            public:
                explicit Ticket(std::uint32_t n) : n_{n} {}

                static constexpr std::uint32_t Ticket::* n_address() { return &Ticket::n_; }

            private:
                std::uint32_t n_ = 0;
            };

            /* never reflected as a wrapper */
            struct Plain {
                int x_;
            };

            void reflect_ticket() {
                WrapperReflector<Ticket> wr;

                if (wr.is_incomplete())
                    wr.reflect_wrapped(Ticket::n_address());
            }
        } /*namespace*/

        TEST_CASE("wrapper-reflects-as-an-atomic", "[reflect][wrapper]") {
            reflect_ticket();

            TypeDescr td = Reflect::require<Ticket>();

            /* a scalar to a consumer: no children */
            REQUIRE(td->metatype() == Metatype::mt_atomic);
            REQUIRE(td->n_child_fixed() == 0);
            REQUIRE(td->is_wrapper());
            REQUIRE(!td->is_enum());
            REQUIRE(!td->is_std_atomic());

            /* standing for its uint32 */
            REQUIRE(td->wrapper_info()->wrapped_td() == Reflect::require<std::uint32_t>());
        }

        TEST_CASE("wrapper-reaches-its-value-in-place", "[reflect][wrapper]") {
            reflect_ticket();

            Ticket t{17};

            WrapperTdx const * wi = Reflect::require<Ticket>()->wrapper_info();
            TaggedPtr v = wi->wrapped_tp(&t);

            REQUIRE(v.td() == Reflect::require<std::uint32_t>());
            REQUIRE(*v.recover_native<std::uint32_t>() == 17);
            /* in place: the member itself, not a copy */
            REQUIRE(v.address() == static_cast<void *>(&(t.*Ticket::n_address())));
        }

        TEST_CASE("wrapper-reflected-once", "[reflect][wrapper]") {
            reflect_ticket();
            reflect_ticket();

            REQUIRE(Reflect::require<Ticket>()->is_wrapper());
        }

        TEST_CASE("plain-struct-is-not-a-wrapper", "[reflect][wrapper]") {
            TypeDescr td = Reflect::require<Plain>();

            REQUIRE(!td->is_wrapper());
            REQUIRE(td->wrapper_info() == nullptr);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end WrapperReflector.test.cpp */
