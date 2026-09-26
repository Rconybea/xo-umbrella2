# xo-pyreactor2websock

Python bindings for [xo-reactor2websock](../xo-reactor2websock), as module
`xo.reactor2websock`.

```python
import xo.reactor2websock as r2w

web.register_stream_endpoint(r2w.stream_endpoint_descr(src, "/ws/uls"))
web.register_http_endpoint(r2w.http_endpoint_descr(store, "/uls"))   # serves /uls/snap
```

Until 2026-09-26 these were methods: `src.stream_endpoint_descr(prefix)` on
`xo.reactor.AbstractSource`, and `store.http_endpoint_descr(prefix)` on
`xo.reactor.AbstractEventStore`. They moved so that xo-reactor no longer depends
on the web layer. See `.xo-backlog/xo-websock/issues/02`.
