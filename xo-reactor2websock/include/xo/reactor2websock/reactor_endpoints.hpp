/** @file reactor_endpoints.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  Web endpoints that expose reactor state: a websocket stream for a source,
 *  and an http snapshot for an event store.
 *
 *  Both were member functions of xo-reactor classes until 2026-09-26
 *  (AbstractSource::stream_endpoint_descr,
 *  AbstractEventStore::http_endpoint_descr), which made xo-reactor depend on
 *  xo-webutil.  Moved here, one level above both xo-reactor and xo-websock, so
 *  the reactor no longer knows the web exists.  See
 *  .xo-backlog/xo-websock/issues/02.
 *
 *  Both take their subject by rp<> and the endpoint holds it.  The member
 *  versions captured a raw `this', so an endpoint registered with a webserver
 *  could outlive the source or store it pointed at.
 **/

#pragma once

#include <xo/webutil/StreamEndpointDescr.hpp>
#include <xo/webutil/HttpEndpointDescr.hpp>
#include <xo/refcnt/Refcounted.hpp>
#include <string>

namespace xo {
    namespace json { class PrintJson; }
    namespace reactor {
        class AbstractSource;
        class AbstractEventStore;
    }

    namespace web {
        /** websocket stream endpoint delivering the events @p src produces.
         *
         *  A subscription through this endpoint attaches a ReactorWebsocketSink
         *  to @p src, forwarding each event to the subscriber's websocket;
         *  unsubscribing detaches it.
         *
         *  @p url_prefix  the endpoint's uri pattern, e.g. "/ws/uls"
         **/
        StreamEndpointDescr stream_endpoint_descr(rp<reactor::AbstractSource> const & src,
                                                  std::string const & url_prefix);

        /** http endpoint serving a json snapshot of @p store, at
         *  @p url_prefix + "/snap".
         *
         *  WARNING: the snapshot is taken on the webserver's service thread,
         *  and nothing here stops the store changing underneath it -- carried
         *  over unchanged from the member version, which said so in 2022.
         *  Holding the store by rp<> fixes the LIFETIME hazard, not this race.
         *
         *  @p pjson  renders the events; the endpoint holds it too
         **/
        HttpEndpointDescr http_endpoint_descr(rp<reactor::AbstractEventStore> const & store,
                                              rp<json::PrintJson> const & pjson,
                                              std::string const & url_prefix);
    } /*namespace web*/
} /*namespace xo*/

/* end reactor_endpoints.hpp */
