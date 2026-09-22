/** @file typeseq.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "xo/reflectutil/typeseq.hpp"

#include <vector>
#include <string>
#include <mutex>

namespace xo {
    namespace reflect {
        namespace {

            /** guards both of the above.
             *
             *  `the upgrade precedes threads' is the model for the phase-(b)
             *  hashmap, but it says nothing about phase (a): a magic static in
             *  two threads can reach two DIFFERENT types' first id draw
             *  concurrently, and they now share one counter where before each
             *  module had its own.  So consolidating the counter is exactly
             *  what makes the race reachable across modules.  Paid once per
             *  (type, module).
             **/
            std::mutex & s_mutex() {
                static std::mutex s_m;
                return s_m;
            }

            /** true for a type with internal linkage.
             *  Such types not be name-keyed.
             *
             *  Two TUs' anonymous types share a spelling and are different types:
             *
             *  @code
             *  TU a:  namespace { struct Widget { int a; }; }       -> "{anonymous}::Widget"
             *  TU b:  namespace { struct Widget { double x, y; }; } -> "{anonymous}::Widget"
             *  @endcode
             *
             *  Keying those together would hand them one id, which is WORSE
             *  than the bug this table fixes.  They draw from the counter and
             *  are not inserted, which is what they did before it existed --
             *  correct for them, since a type with internal linkage cannot be
             *  the same type in two modules and so needs no global id.
             *
             *  Both spellings: gcc says @c {anonymous}, clang says
             *  @c (anonymous namespace).
             **/
            bool has_internal_linkage(std::string_view name) {
                return ((name.find("{anonymous}") != std::string_view::npos)
                        || (name.find("(anonymous namespace)") != std::string_view::npos));
            }
        } /*namespace*/

        int32_t
        typerecd::s_next_id = 0;

        std::vector<typerecd>
        typerecd::s_typerecd_table_;

        typerecd
        typerecd::_by_name(std::string_view name)
        {
            std::lock_guard<std::mutex> lock(s_mutex());

            if (has_internal_linkage(name)) {
                /* generate an id, but do not (and must not) insert */
                return typerecd(s_next_id++, name);
            }

            for (const typerecd & ix : s_typerecd_table_) {
                if (ix.name_ == name)
                    return ix;
            }

            /* ids are dense and consecutive.
             * TypeRegistry indexes a vector by seqno().
             * Note the id is NOT the table index: an anonymous type
             * draws from the counter without adding a row, so the two diverge.
             */
            std::int32_t id = s_next_id++;

            s_typerecd_table_.push_back(typerecd{id, name});

            return typerecd{id, name};
        }

        std::int32_t
        typerecd::_id_count()
        {
            return s_next_id;
        }

        std::size_t
        typerecd::_table_z()
        {
            return s_typerecd_table_.size();
        }

    } /*namespace reflect*/
} /*namespace xo*/

/* end typeseq.cpp */
