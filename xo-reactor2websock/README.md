# xo-reactor2websock

Adapter between [xo-reactor](../xo-reactor) (typed event sources and sinks) and
[xo-websock](../xo-websock) (websocket transport). Read the name as
reactor-*to*-websock; it is not about xo-reactor2.

It exists so that neither of those depends on the other:

- xo-websock carries frames. It speaks only `TaggedPtr`, and renders with
  PrintJson.
- xo-reactor no longer knows the web exists.

Provides:

- `xo::web::ReactorWebsocketSink`: a `reactor::AbstractSink` that forwards
  every event to a `web::WebsocketSink`.
- `xo::web::stream_endpoint_descr(src, prefix)`: a websocket stream endpoint for
  a reactor source.
- `xo::web::http_endpoint_descr(store, pjson, prefix)`: an http snapshot
  endpoint (`prefix + "/snap"`) for a reactor event store.

Both endpoint builders hold their subject by `rp<>`. Until 2026-09-26 they were
member functions of xo-reactor classes, and those captured a raw `this`.

Python bindings: [xo-pyreactor2websock](../xo-pyreactor2websock).

See `.xo-backlog/xo-websock/issues/02`.
