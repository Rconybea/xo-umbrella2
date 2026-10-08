/** @file Guard.test.cpp
 *
 *  lockables and guard declarations -- see .xo-backlog/xo-reflect/issues/08.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/reflect/StructReflector.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/struct/GuardedVisit.hpp"
#include <catch2/catch.hpp>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace xo {
    using xo::reflect::GuardMode;
    using xo::reflect::LockableTdx;
    using xo::reflect::Metatype;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TypeDescr;
    using xo::reflect::visit_members_guarded;

    namespace ut {
        namespace {
            /* run @p fn on another thread; its result.  Probes a lockable
             * this thread may hold: try_lock on a mutex the caller owns is
             * undefined
             */
            template <typename Fn>
            bool on_other_thread(Fn fn) {
                bool retval = false;
                std::thread t([&retval, &fn]() { retval = fn(); });
                t.join();
                return retval;
            }

            /* true iff another thread could take @p m exclusively now */
            template <typename Mutex>
            bool free_elsewhere(Mutex & m) {
                return on_other_thread([&m]() {
                    if (m.try_lock()) {
                        m.unlock();
                        return true;
                    }
                    return false;
                });
            }

            /* true iff another thread could take @p m shared now */
            bool shareable_elsewhere(std::shared_mutex & m) {
                return on_other_thread([&m]() {
                    if (m.try_lock_shared()) {
                        m.unlock_shared();
                        return true;
                    }
                    return false;
                });
            }

            /* two guards: m1 guards a and c, m2 guards b; free_ unguarded */
            struct Guarded {
                int free_ = 0;
                int a_ = 0;
                int b_ = 0;
                int c_ = 0;
                mutable std::mutex m1_;
                mutable std::shared_mutex m2_;
            };

            void reflect_guarded() {
                StructReflector<Guarded> sr;

                if (sr.is_incomplete()) {
                    REFLECT_MEMBER(sr, free);
                    REFLECT_MEMBER(sr, a).guarded_by(&Guarded::m1_);
                    REFLECT_MEMBER(sr, b).guarded_by(&Guarded::m2_);
                    REFLECT_MEMBER(sr, c).guarded_by(&Guarded::m1_);
                }
            }

            /* adopts Guarded's guards after declaring one of its own */
            struct DerivedGuarded : public Guarded {
                int d_ = 0;
                std::mutex m3_;
            };

            void reflect_derived_guarded() {
                reflect_guarded();

                StructReflector<DerivedGuarded> sr;

                if (sr.is_incomplete()) {
                    REFLECT_MEMBER(sr, d).guarded_by(&DerivedGuarded::m3_);
                    sr.adopt_ancestors<Guarded>();
                }
            }

            /* index of member @p name in reflected struct @p td */
            uint32_t member_ix(TypeDescr td, std::string const & name) {
                for (uint32_t i = 0, n = td->n_child(nullptr); i < n; ++i) {
                    if (td->struct_member(i).member_name() == name)
                        return i;
                }

                FAIL("no member " << name);
                return 0;
            }

            /* guard index of member @p name; -1 if unguarded */
            int guard_of(TypeDescr td, std::string const & name) {
                auto g = td->struct_member(member_ix(td, name)).guard_ix();

                return g ? static_cast<int>(*g) : -1;
            }
        } /*namespace*/

        TEST_CASE("lockable-std-mutex", "[reflect][guard]") {
            TypeDescr td = Reflect::require<std::mutex>();

            REQUIRE(td->is_lockable());
            REQUIRE(td->metatype() == Metatype::mt_atomic);
            REQUIRE(td->n_child_fixed() == 0);

            std::mutex m;
            LockableTdx const * lk = td->lockable_info();

            lk->read_lock(&m);
            /* exclusive: nobody else gets in */
            REQUIRE(!free_elsewhere(m));
            lk->read_unlock(&m);
            REQUIRE(free_elsewhere(m));

            REQUIRE(lk->try_read_lock(&m));
            REQUIRE(!free_elsewhere(m));
            lk->read_unlock(&m);
        }

        TEST_CASE("lockable-std-shared-mutex", "[reflect][guard]") {
            TypeDescr td = Reflect::require<std::shared_mutex>();

            REQUIRE(td->is_lockable());

            std::shared_mutex m;
            LockableTdx const * lk = td->lockable_info();

            lk->read_lock(&m);
            /* shared: other readers get in, a writer does not */
            REQUIRE(shareable_elsewhere(m));
            REQUIRE(!free_elsewhere(m));
            lk->read_unlock(&m);
            REQUIRE(free_elsewhere(m));
        }

        TEST_CASE("lockable-not-for-other-types", "[reflect][guard]") {
            REQUIRE(!Reflect::require<int>()->is_lockable());
            REQUIRE(Reflect::require<int>()->lockable_info() == nullptr);
        }

        TEST_CASE("guard-interning", "[reflect][guard]") {
            reflect_guarded();

            TypeDescr td = Reflect::require<Guarded>();

            /* one guard per distinct lockable */
            REQUIRE(td->n_guard() == 2);
            REQUIRE(guard_of(td, "free") == -1);
            REQUIRE(guard_of(td, "a") == 0);
            REQUIRE(guard_of(td, "b") == 1);
            REQUIRE(guard_of(td, "c") == 0);

            /* each guard reaches its lockable */
            Guarded x;
            REQUIRE(td->guard_tp(0, &x).address() == static_cast<void *>(&x.m1_));
            REQUIRE(td->guard_tp(1, &x).address() == static_cast<void *>(&x.m2_));
            REQUIRE(td->guard_tp(0, &x).td()->is_lockable());
        }

        TEST_CASE("guard-visit-order-and-holding", "[reflect][guard]") {
            reflect_guarded();

            TypeDescr td = Reflect::require<Guarded>();
            Guarded x;

            std::vector<uint32_t> order;
            /* per member: was m1, m2 free to others during its visit? */
            std::vector<bool> m1_free, m2_free, m2_shareable;

            visit_members_guarded(td, &x, GuardMode::blocking,
                                  [&](uint32_t i, bool readable) {
                                      REQUIRE(readable);
                                      order.push_back(i);
                                      m1_free.push_back(free_elsewhere(x.m1_));
                                      m2_free.push_back(free_elsewhere(x.m2_));
                                      m2_shareable.push_back(shareable_elsewhere(x.m2_));
                                  });

            /* unguarded first, then m1's group, then m2's */
            REQUIRE(order == std::vector<uint32_t>{member_ix(td, "free"),
                                                   member_ix(td, "a"), member_ix(td, "c"),
                                                   member_ix(td, "b")});
            /* free: nothing held */
            REQUIRE(m1_free[0]);
            REQUIRE(m2_free[0]);
            /* a, c: m1 held, m2 not -- sibling guards never nest */
            REQUIRE(!m1_free[1]);
            REQUIRE(!m1_free[2]);
            REQUIRE(m2_free[1]);
            REQUIRE(m2_free[2]);
            /* b: m2 held shared, m1 released */
            REQUIRE(m1_free[3]);
            REQUIRE(!m2_free[3]);
            REQUIRE(m2_shareable[3]);

            /* all released afterwards */
            REQUIRE(free_elsewhere(x.m1_));
            REQUIRE(free_elsewhere(x.m2_));
        }

        TEST_CASE("guard-visit-try-mode", "[reflect][guard]") {
            reflect_guarded();

            TypeDescr td = Reflect::require<Guarded>();
            Guarded x;

            std::vector<std::pair<uint32_t, bool>> seen;
            {
                /* this thread holds m1; the visit runs elsewhere */
                std::lock_guard<std::mutex> lock(x.m1_);

                std::thread t([&]() {
                    visit_members_guarded(td, &x, GuardMode::try_lock,
                                          [&seen](uint32_t i, bool readable) {
                                              seen.emplace_back(i, readable);
                                          });
                });
                t.join();
            }

            REQUIRE(seen == std::vector<std::pair<uint32_t, bool>>{
                    {member_ix(td, "free"), true},
                    {member_ix(td, "a"), false},
                    {member_ix(td, "c"), false},
                    {member_ix(td, "b"), true}});
        }

        TEST_CASE("guard-released-on-exception", "[reflect][guard]") {
            reflect_guarded();

            TypeDescr td = Reflect::require<Guarded>();
            Guarded x;
            uint32_t a_ix = member_ix(td, "a");

            REQUIRE_THROWS_AS(visit_members_guarded(td, &x, GuardMode::blocking,
                                                    [a_ix](uint32_t i, bool) {
                                                        if (i == a_ix)
                                                            throw std::runtime_error("from fn");
                                                    }),
                              std::runtime_error);

            REQUIRE(free_elsewhere(x.m1_));
            REQUIRE(free_elsewhere(x.m2_));
        }

        TEST_CASE("guard-adopt-ancestors", "[reflect][guard]") {
            reflect_derived_guarded();

            TypeDescr td = Reflect::require<DerivedGuarded>();

            /* its own guard first, then the ancestor's two, shifted */
            REQUIRE(td->n_guard() == 3);
            REQUIRE(guard_of(td, "d") == 0);
            REQUIRE(guard_of(td, "free") == -1);
            REQUIRE(guard_of(td, "a") == 1);
            REQUIRE(guard_of(td, "b") == 2);
            REQUIRE(guard_of(td, "c") == 1);

            DerivedGuarded x;
            REQUIRE(td->guard_tp(0, &x).address() == static_cast<void *>(&x.m3_));
            REQUIRE(td->guard_tp(1, &x).address() == static_cast<void *>(&x.m1_));
            REQUIRE(td->guard_tp(2, &x).address() == static_cast<void *>(&x.m2_));

            /* and a visit holds the adopted guard */
            bool m1_held_at_a = false;
            uint32_t a_ix = member_ix(td, "a");

            visit_members_guarded(td, &x, GuardMode::blocking,
                                  [&](uint32_t i, bool) {
                                      if (i == a_ix)
                                          m1_held_at_a = !free_elsewhere(x.m1_);
                                  });

            REQUIRE(m1_held_at_a);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end Guard.test.cpp */
