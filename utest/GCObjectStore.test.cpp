/** @file GCObjectStore.test.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include <xo/object2/List.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/gc/GCObjectStore.hpp>
#include <xo/alloc2/GCObject.hpp>
#include <xo/indentlog/scope.hpp>
#include <catch2/catch.hpp>

namespace ut {
    using xo::scm::DList;
    using xo::scm::DInteger;
    using xo::mm::GCObjectStoreConfig;
    using xo::mm::GCObjectStore;
    using xo::mm::AGCObject;
    using xo::mm::ArenaConfig;
    using xo::facet::typeseq;
    using xo::facet::impl_for;
    using xo::scope;
    using std::size_t;
    using std::uint32_t;

    namespace {
        struct Testcase {
            explicit Testcase(uint32_t n_gen, uint32_t n_survive,
                              size_t gc_z, uint32_t type_z)
                : n_gen_{n_gen},
                  n_survive_{n_survive},
                  gc_size_{gc_z},
                  object_type_z_{type_z}
                {}

            /** number of generations in gco store **/
            uint32_t n_gen_ = 0;
            /** object promotes on surviving this many gc cycles **/
            uint32_t n_survive_ = 0;
            /** size of each generation's half-space, in bytes **/
            size_t gc_size_ = 0;
            /** Storage for object type array, in bytes.
             *  (need to allow 1 pointer per type)
             **/
            uint32_t object_type_z_ = 0;

        };

        static std::vector<Testcase> s_testcase_v = {
            /** n_gen, n_survive, gc_size, object_type_z **/
            Testcase(2, 4, 16 * 1024, 8 * 128),
        };
    }

    TEST_CASE("GCObjectStore-1", "[GCObjectStore]")
    {
        constexpr bool c_debug_flag = false;
        scope log(XO_DEBUG(c_debug_flag));

        for (size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            const Testcase & tc = s_testcase_v[i_tc];

            /** config for each half-space **/
            ArenaConfig arena_config
                = (ArenaConfig()
                   .with_name("arena-name-not-used")
                   .with_size(tc.gc_size_)
                   .with_store_header_flag(true));

            GCObjectStoreConfig gcos_config(arena_config,
                                            tc.n_gen_,
                                            tc.n_survive_,
                                            tc.object_type_z_,
                                            c_debug_flag);

            // object type storage will be empty unless we install a type.
            GCObjectStore gcos(gcos_config);

            REQUIRE(gcos.is_type_installed(typeseq::id<DList>()) == false);

            //REQUIRE(nullptr == gcos.lookup_type(typeseq::id<DList>()));

#ifdef NOT_YET


//            // Usual path would be via ACollector interface; that's inconvenient here
//
#endif
        }
    }

} /*namespace ut*/

/* end GCObjectStore.test.cpp */
