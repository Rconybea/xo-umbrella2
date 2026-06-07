/* @file AbstractEventProcessor.cp */

#include "AbstractEventProcessor.hpp"
#include "xo/indentlog/print/tostr.hpp"
#include <unordered_map>
#include <map>

namespace xo {
    using xo::tostr;
    using std::uint32_t;

    namespace reactor {
        namespace {
            /* search all event processors ep reachable (dowstream) from x,
             * add to *m;
             */
            void
            map_network_helper(bp<AbstractEventProcessor> x,
                               uint32_t * tsort_ix,
                               std::unordered_map<AbstractEventProcessor*, uint32_t> * m)
            {
                if (m->contains(x.get()))
                    return;

                auto fn = [tsort_ix, m]
                    (bp<AbstractEventProcessor> ep)
                    {
                        map_network_helper(ep, tsort_ix, m);
                    };

                x->visit_direct_consumers(fn);

                /* postorder! */
                (*m)[x.get()] = ++(*tsort_ix);

            } /*map_network_helper*/
        } /*namespace*/

        std::vector<rp<AbstractEventProcessor>>
        AbstractEventProcessor::map_network(rp<AbstractEventProcessor> const & x)
        {
            std::unordered_map<AbstractEventProcessor *, std::uint32_t> network_map;

            /* index event processors in reverse topological order:
             * if B is (directly or indirectly) downstream from A,
             * then tsort_ix(B) < tsort_ix(A)
             */
            uint32_t tsort_ix = 0;

            /* depth-first traversal,  detect and short-circuit on dup paths */
            map_network_helper(x.borrow(), &tsort_ix, &network_map);

            /* invariant: tsort_ix = #of event processors in network */
            uint32_t n = tsort_ix;

            /* network_map, now in a topologically sorted order */
            std::map<uint32_t, AbstractEventProcessor *> tsorted_map;
            {
                for(auto const & x : network_map) {
                    uint32_t tsort_ix = x.second;
                    AbstractEventProcessor * ep = x.first;

                    tsorted_map[n - tsort_ix] = ep;
                }
            }

            std::vector<rp<AbstractEventProcessor>> retval;
            {
                for(auto const & x : tsorted_map)
                    retval.push_back(x.second);
            }

            return retval;
        } /*map_network*/

        void
        AbstractEventProcessor::display(std::ostream & os) const
        {
            os << "<AbstractEventProcessor" << xtag("name", name()) << ">";
        } /*display*/

        std::string
        AbstractEventProcessor::display_string() const
        {
            return tostr(*this);
        } /*display_string*/

    } /*namespace reactor*/
} /*namespace xo*/

/* end AbstractEventProcessor.cpp */
