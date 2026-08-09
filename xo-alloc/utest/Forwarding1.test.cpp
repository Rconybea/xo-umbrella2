/* Forwarding1.test.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "Forwarding1.hpp"
#include "ArenaAlloc.hpp"
#include "alloc_ostream.hpp"           /* os << gp<Object> */
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/pretty_array.hpp>   /* Prettifier<std::array<T,N>> */
#include <xo/ppsink/pretty_ostream.hpp> /* pp_to_stream */
#include <catch2/catch.hpp>
#include <cstring>
#include <regex>

namespace xo {
    using xo::reflect::Reflect;
    using xo::obj::Forwarding1;

    namespace gc {
        namespace {
            class DummyObject : public Object {
            public:
                explicit DummyObject(const char * data) {
                    ::strncpy(data_.data(), data, 128);
                }

                gp<Object> member() const { return member_; }
                void assign_member(Object * x) {
                    Object::mm->assign_member(this, reinterpret_cast<IObject**>(member_.ptr_address()), x);
                }

                TaggedPtr self_tp() const final override {
                    return Reflect::make_tp(const_cast<DummyObject*>(this));
                }

                /* legacy print/array.hpp supplied operator<<(ostream, std::array);
                 * ppsink has Prettifier<std::array<T,N>> instead, reached from an
                 * ostream via pp_to_stream().
                 */
                void pretty(xo::pp::PpSink & sink) const final override {
                    /* was pp_to_stream(os, data_) -- the ostream adapter for
                     * Prettifier<std::array<T,N>>.  With a sink in hand the
                     * adapter is unnecessary: hand the array straight over.
                     */
                    sink.pp(data_);
                }

                virtual std::size_t _shallow_size() const final override { return sizeof(*this); }
                virtual IObject * _shallow_copy(gc::IAlloc * mm) const final override { return new (Cpof(mm, this)) DummyObject(*this); }
                virtual std::size_t _forward_children(gc::IAlloc * gc) final override { return _shallow_size(); }

            private:
                std::array<char, 128> data_;
                gp<Object> member_;
            };
        }

        TEST_CASE("Forwarding1", "[gc][alloc]")
        {
            gp<Object> obj = new DummyObject("Well, I wasn't expecting that!");
            gp<Forwarding1> fwd = new Forwarding1(obj);

            REQUIRE(fwd->_destination() == obj.ptr());
            REQUIRE(fwd->_offset_destination(fwd.ptr()) == obj.ptr());

            REQUIRE(fwd->self_tp().td()->short_name() == "Forwarding1");

            std::stringstream ss;
            ss << fwd;

            // forwarding printer looks like
            //  "<fwd :dest 0x1ef49c20>"
            //

            std::regex pattern(R"(<fwd :dest 0x[0-9a-f]+>)");
            REQUIRE(std::regex_match(ss.str(), pattern));

            //REQUIRE(ss.str() == "<fwd :dest DummyObject>");

        }

        TEST_CASE("IAlloc.assign_member", "[gc][alloc]")
        {
            /* not giving this nit it's own translation unit.
             */

            gp<DummyObject> obj = new DummyObject("This also a surprise..");

            up<ArenaAlloc> arena = ArenaAlloc::make("test", 1024, false);

            Object::mm = arena.get();

            obj->assign_member(obj.ptr());

            REQUIRE(obj->member().ptr() == obj.ptr());
        }

    } /*namespace gc*/
} /*namespace xo*/

/* end Forwarding1.test.cpp */
