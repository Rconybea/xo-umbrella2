/* @file AbstractSource.cpp */

#include "AbstractSource.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/webutil/StreamEndpointDescr.hpp"
//#include "indentlog/scope.hpp"

namespace xo {
    using xo::web::StreamEndpointDescr;
    using xo::reactor::AbstractSink;

    namespace reactor {
        StreamEndpointDescr
        AbstractSource::stream_endpoint_descr(std::string const & url_prefix)
        {
            auto subscribe_fn
                = ([this]
                   (rp<AbstractSink> const & ws_sink)
                    {
                        //scope lscope("AbstractSource::stream_endpoint_descr.subscribe_fn");

                        /* ws_sink created by websocket,  sends events to websocket as json
                         * see [websock/WebsocketSink]
                         */
                        return this->attach_sink(ws_sink);
                    });

            auto unsubscribe_fn
                = ([this]
                   (CallbackId id)
                    {
                        this->detach_sink(id);
                    });

            return StreamEndpointDescr(url_prefix,
                                       subscribe_fn,
                                       unsubscribe_fn);
        } /*stream_endpoint_descr*/

        uint64_t
        AbstractSource::deliver_n(uint64_t n)
        {
            uint64_t retval = 0;

            for (uint64_t i=0; i<n; ++i) {
                uint64_t n1 = this->deliver_one();

                if (n1 == 0) {
                    /* short-circuit if source has less than n
                     * events available
                     */
                    break;
                }

                retval += n1;
            }

            return retval;
        } /*deliver_n*/

        uint64_t
        AbstractSource::deliver_all()
        {
            uint64_t retval = 0;

            for (;;) {
                uint64_t n1 = this->deliver_one();

                if (n1 == 0)
                    break;

                retval += n1;
            }

            return retval;
        } /*deliver_all*/

    } /*namespace reactor*/
} /*namespace xo*/

/* end AbstractSource.cpp */
