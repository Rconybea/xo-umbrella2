/** @file reactor_endpoints.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "reactor_endpoints.hpp"
#include "ReactorWebsocketSink.hpp"
#include <xo/reactor/AbstractSource.hpp>
#include <xo/reactor/EventStore.hpp>
#include <xo/printjson/PrintJson.hpp>
#include <xo/webutil/Alist.hpp>

namespace xo {
    using xo::reactor::AbstractSink;
    using xo::reactor::AbstractSource;
    using xo::reactor::AbstractEventStore;
    using xo::json::PrintJson;
    using xo::fn::CallbackId;

    namespace web {
        StreamEndpointDescr
        stream_endpoint_descr(rp<AbstractSource> const & src,
                              std::string const & url_prefix)
        {
            auto subscribe_fn
                = ([src](rp<WebsocketSink> const & ws_sink)
                    {
                        /* ws_sink is created by the webserver per subscription,
                         * and sends events to that subscription's websocket as
                         * json.  The adapter is what lets a reactor source
                         * deliver into it.
                         */
                        rp<AbstractSink> sink = ReactorWebsocketSink::make(ws_sink);

                        return src->attach_sink(sink);
                    });

            auto unsubscribe_fn
                = ([src](CallbackId id)
                    {
                        src->detach_sink(id);
                    });

            return StreamEndpointDescr(url_prefix,
                                       subscribe_fn,
                                       unsubscribe_fn);
        } /*stream_endpoint_descr*/

        HttpEndpointDescr
        http_endpoint_descr(rp<AbstractEventStore> const & store,
                            rp<PrintJson> const & pjson,
                            std::string const & url_prefix)
        {
            /* the lambda keeps its own references: the endpoint may be invoked
             * long after this call returns
             */
            auto http_fn = ([store, pjson]
                            (std::string const & /*uri*/,
                             Alist const & /*alist*/,
                             std::ostream * p_os)
                {
                    /* WARNING: race condition here, given webserver runs from
                     *          a separate thread.  See header.
                     */
                    store->http_snapshot(pjson, p_os);
                });

            return HttpEndpointDescr(url_prefix + "/snap", http_fn);
        } /*http_endpoint_descr*/
    } /*namespace web*/
} /*namespace xo*/

/* end reactor_endpoints.cpp */
