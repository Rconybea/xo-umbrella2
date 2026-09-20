/** @file reflect_flywheel_info.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "xo/object2/reflect_flywheel_info.hpp"
#include <xo/facet/FlywheelInfo.hpp>
#include <xo/arena/MemorySizeInfo.hpp>
#include <xo/reflect/StructReflector.hpp>

namespace xo {
    using xo::reflect::StructReflector;

    using xo::mm::MemorySizeInfo;

    namespace facet {
        void
        reflect_flywheel_info()
        {
            /* Member names are given EXPLICITLY rather than through
             * REFLECT_MEMBER, which would derive the json key from the c++
             * member name (`pool_v_' -> "pool_v").
             *
             * That decoupling is the point of a view model: the keys below are
             * a wire contract with a browser, and house style for a member
             * holding a vector should not be able to rename them.  The cost is
             * that a member rename does not automatically rename its key --
             * which is the intended direction, and why the frame test asserts
             * the key names rather than deriving them.
             */
            {
                /* MemorySizeInfo itself, not a shadow copy of it.
                 *
                 * detail_ is DELIBERATELY absent: it is a pointer into the
                 * stack frame of whoever ran the visit, so following it from a
                 * snapshot taken later is a dangling read.  It is also almost
                 * always null.  StructReflector does not require a member list
                 * to account for sizeof, so omitting it is simply omitting it;
                 * if the per-type histogram is ever wanted, it belongs in its
                 * own report at its own cadence.
                 */
                StructReflector<MemorySizeInfo> sr;

                sr.reflect_member("name", &MemorySizeInfo::resource_name_);
                sr.reflect_member("used", &MemorySizeInfo::used_);
                sr.reflect_member("allocated", &MemorySizeInfo::allocated_);
                sr.reflect_member("committed", &MemorySizeInfo::committed_);
                sr.reflect_member("reserved", &MemorySizeInfo::reserved_);
                sr.reflect_member("lo", &MemorySizeInfo::lo_);
                sr.reflect_member("hi", &MemorySizeInfo::hi_);

                sr.require_complete();
            }

            /* no StructReflector for the root set.  It used to be a
             * RootSetInfo -- five fields copied out of the store and described
             * here a third time -- retired 2026-09-21.  FlywheelInfo::strong_
             * is now a pointer to the live store, and JsonPrinter_RootSet
             * reads it (and the ObjectSlots inside it) directly.
             *
             * Nothing to register for that member: a raw pointer reflects as
             * an atom, which is exactly what the printer keys on.
             */

            {
                StructReflector<FlywheelInfo> sr;

                sr.reflect_member("pools", &FlywheelInfo::pool_v_);
                sr.reflect_member("strong", &FlywheelInfo::strong_);

                sr.require_complete();
            }
        } /*reflect_flywheel_info*/
    } /*namespace facet*/
} /*namespace xo*/

/* end reflect_flywheel_info.cpp */
