// introspect.js -- page side of xo-websock/example/introspect
//
// Protocol (xo-websock, .xo-backlog/xo-websock/issues/04, 06):
//   -> {"cmd": "subscribe", "stream": "/introspect"}
//   <- {"cmd": "subscribed", "stream": "/introspect", "sub_id": N}
//   -> {"cmd": "send", "sub_id": N, "msg": "refresh"}
//   <- {"stream": "/introspect", "sub_id": N, "seq": k,
//       "event": {"server": <Webserver json>, "ticker": <Ticker json>}}
//
// Webserver json: {id, refcount, listen_port, state,
//            endpoints: [{id, refcount, kind, stem, pattern, has_receive}],
//            sessions: [{id, session_id,
//                        sender: {id, refcount, session_id, open},
//                        subscriptions: [{id, sub_id, stream,
//                                         endpoint: {ref},
//                                         sink: {id, refcount, stream, sub_id,
//                                                seq, sender: {ref}}}]}]}.
//
// Each object is printed in full once; elsewhere as {ref: id}.  The page
// joins refs to objects by id.
//
// Ticker json: {id, sinks: [{ref}]} -- the application's own holds.
//
// Source links (.xo-backlog/xo-websock/issues/12): every object carries
// _type_, its C++ type's canonical name.  /dyn/types maps a type -- template
// arguments stripped -- to {file, line}, and says how to link one ("link":
// a url template with {file}, {line}; null for none).  Hover a box for its
// type and location; click to open the source.
//
// Refcount accounting: each refcount is compared with the holds the page can
// see.  An endpoint is held by the router's map + each subscription to it; a
// sender by its session record + router + each sink; a sink by the router's
// slot + each application ref (the ticker).  More than that is flagged: a
// hold the snapshot does not show.

"use strict";

const status_el = document.getElementById("status");
const raw_el = document.getElementById("raw");
const refresh_btn = document.getElementById("refresh");
const srcinfo_el = document.getElementById("srcinfo");

let sub_id = null;
let last_event = null;

// the type -> source map and link template, from /dyn/types
let src = {types: {}, link: null, n_maps: 0};

function load_types() {
    fetch("/dyn/types")
        .then(r => r.json())
        .then(j => {
            src = {types: j.types || {}, link: j.link || null,
                   n_maps: (j.subsystems || []).length};
            const n = Object.keys(src.types).length;
            srcinfo_el.textContent =
                (n === 0)
                ? "source links: none -- no type maps (configure with --enable-source-map)"
                : (src.link === null)
                ? `source links: off -- ${n} types mapped; start with --src-tree=ROOT or --src-link=TEMPLATE`
                : `source links: ${src.link} -- ${n} types from ${src.n_maps} subsystems`;
            if (last_event)
                draw(last_event);
        })
        .catch(e => { srcinfo_el.textContent = `source links: /dyn/types failed: ${e}`; });
}

/** {file, line, href} for canonical type name @p t, or null if unmapped **/
function source_of(t) {
    if (!t)
        return null;

    // a template maps under its bare name
    const loc = src.types[t.replace(/<.*$/, "")];
    if (!loc)
        return null;

    const href = src.link && src.link
          .replace("{file}", loc.file.split("/").map(encodeURIComponent).join("/"))
          .replace("{line}", loc.line);

    return {file: loc.file, line: loc.line, href: href};
}

const ws = new WebSocket(`ws://${location.host}/`, "lws-minimal");

ws.onopen = () => {
    status_el.textContent = "connected; subscribing...";
    ws.send(JSON.stringify({cmd: "subscribe", stream: "/introspect"}));
};

ws.onclose = () => {
    status_el.textContent = "disconnected";
    refresh_btn.disabled = true;
};

ws.onmessage = (ev) => {
    const msg = JSON.parse(ev.data);

    if (msg.cmd === "subscribed") {
        sub_id = msg.sub_id;
        status_el.textContent = `subscribed (sub_id ${sub_id})`;
        refresh_btn.disabled = false;
        load_types();
        refresh();
    } else if (msg.error) {
        status_el.textContent = `error: ${msg.error}`;
    } else if ("event" in msg) {
        raw_el.textContent = JSON.stringify(msg, null, 2);
        last_event = msg.event;
        draw(msg.event);
    }
};

function refresh() {
    // the maps too: a rebuild changes them
    load_types();

    if (sub_id !== null)
        ws.send(JSON.stringify({cmd: "send", sub_id: sub_id, msg: "refresh"}));
}

refresh_btn.onclick = refresh;

// layout: server in the middle, http endpoints to its left, stream endpoints
// to its right -- so no link passes behind another box.  Sessions in a row
// beneath all of it, linked up to the server; each session's subscriptions
// stacked under it, each with a curved "uses" link to its stream endpoint
const col_x = {http: 30, server: 330, stream: 640};
const row_h = 58;
const top_y = 40;

function layout(event) {
    const snap = event.server;
    const ticker = event.ticker;

    const nodes = [];
    const links = [];
    const n_in = {http: 0, stream: 0};

    const endpoint_node = {};   // endpoint object id -> node id

    for (const ep of (snap.endpoints || [])) {
        const id = `${ep.kind}:${ep.stem}`;
        endpoint_node[ep.id] = id;
        nodes.push({id: id, kind: ep.kind, label: ep.pattern, refcount: ep.refcount,
                    type: ep._type_,
                    x: col_x[ep.kind], y: top_y + row_h * n_in[ep.kind]++});
        links.push({source: "server", target: id});
    }

    // server centred on the taller column
    const n_rows = Math.max(1, n_in.http, n_in.stream);
    nodes.unshift({id: "server", kind: "server",
                   label: `Webserver :${snap.listen_port} (${snap.state})`,
                   type: snap._type_,
                   x: col_x.server, y: top_y + row_h * (n_rows - 1) / 2});

    // sessions: one row, below the endpoint columns; subscriptions under each
    const session_y = top_y + row_h * n_rows + 50;
    const sub_h = 44;
    const uses = [];   // subscription -> endpoint
    let n_sub_max = 0;

    (snap.sessions || []).forEach((s, i) => {
        const id = `session:${s.session_id}`;
        const x = col_x.http + i * 240;
        const subs = s.subscriptions || [];
        const open = s.sender && s.sender.open;

        nodes.push({id: id, kind: open ? "session" : "session closed",
                    label: `session ${s.session_id}`, type: s._type_,
                    x: x, y: session_y});
        links.push({source: "server", target: id});

        // the session's sender: first under it, with its refcount
        if (s.sender) {
            const snd = `${id}:sender`;
            nodes.push({id: snd, kind: "sender", label: "sender",
                        refcount: s.sender.refcount, type: s.sender._type_,
                        x: x + 18, y: session_y + row_h, small: true});
            links.push({source: id, target: snd, kind: "owns"});
        }

        subs.forEach((sub, k) => {
            const sid = `${id}:sub:${sub.sub_id}`;
            const sink = sub.sink || {};
            // the sink's sender should be this session's: flag it if not
            const astray = s.sender && sink.sender && sink.sender.ref !== s.sender.id;

            nodes.push({id: sid, kind: astray ? "subscription astray" : "subscription",
                        label: `sub ${sub.sub_id} · ${sub.stream}`,
                        refcount: sink.refcount,   // the sink's: slot + its source
                        type: sub._type_,
                        x: x + 18, y: session_y + row_h + (k + 1) * sub_h, small: true});
            links.push({source: id, target: sid, kind: "owns"});

            // joined BY ID: the endpoint object this subscription holds
            const ep = sub.endpoint && endpoint_node[sub.endpoint.ref];
            if (ep)
                uses.push({source: sid, target: ep});
        });

        n_sub_max = Math.max(n_sub_max, subs.length + 1);   // + the sender
    });

    // the application's ticker, right of the sessions; a "holds" link to
    // each subscription whose sink it refers to
    const holds = [];
    if (ticker) {
        const n_s = (snap.sessions || []).length;
        nodes.push({id: "ticker", kind: "app", label: "Ticker (app)", type: ticker._type_,
                    x: col_x.http + n_s * 240, y: session_y});

        const sub_of_sink = {};
        for (const s of (snap.sessions || []))
            for (const sub of (s.subscriptions || []))
                if (sub.sink) sub_of_sink[sub.sink.id] = `session:${s.session_id}:sub:${sub.sub_id}`;

        for (const r of (ticker.sinks || []))
            if (sub_of_sink[r.ref]) holds.push({source: "ticker", target: sub_of_sink[r.ref]});
    }

    // refcount accounting: the holds this snapshot shows, per object
    const expect = {};   // node id -> expected refcount
    const bump = (k, n) => { expect[k] = (expect[k] || 0) + n; };
    const node_of_ep = endpoint_node;
    for (const ep of (snap.endpoints || []))
        bump(node_of_ep[ep.id], 1);                          // router's map
    const app_refs = {};
    for (const r of ((ticker && ticker.sinks) || []))
        app_refs[r.ref] = (app_refs[r.ref] || 0) + 1;
    for (const s of (snap.sessions || [])) {
        const sid = `session:${s.session_id}`;
        bump(`${sid}:sender`, 2);                            // record + router
        for (const sub of (s.subscriptions || [])) {
            const subn = `${sid}:sub:${sub.sub_id}`;
            if (sub.endpoint) bump(node_of_ep[sub.endpoint.ref], 1);
            if (sub.sink) {
                bump(subn, 1 + (app_refs[sub.sink.id] || 0)); // slot + app
                if (sub.sink.sender && s.sender && sub.sink.sender.ref === s.sender.id)
                    bump(`${sid}:sender`, 1);
            }
        }
    }
    for (const n of nodes)
        if (n.refcount !== undefined) n.expected = expect[n.id] || 0;

    const n_session = (snap.sessions || []).length;
    const height = (n_session > 0
                    ? session_y + row_h + n_sub_max * sub_h
                    : top_y + row_h * n_rows) + 20;

    return {nodes, links, uses, holds, height};
}

function draw(event) {
    const {nodes, links, uses, holds, height} = layout(event);
    const by_id = new Map(nodes.map(d => [d.id, d]));
    const box_h = 40;

    const svg = d3.select("#graph").attr("height", height);

    // two fixed layers, links under boxes -- new elements go into their
    // layer, so a refresh cannot paint a line over a box
    const link_layer = svg.selectAll("g.links").data([0]).join("g").attr("class", "links");
    const uses_layer = svg.selectAll("g.uses").data([0]).join("g").attr("class", "uses");
    const holds_layer = svg.selectAll("g.holds").data([0]).join("g").attr("class", "holds");
    const node_layer = svg.selectAll("g.nodes").data([0]).join("g").attr("class", "nodes");

    // headings
    const session_node = nodes.find(d => d.kind.startsWith("session"));
    const headings = [["http endpoints", col_x.http, 22], ["stream endpoints", col_x.stream, 22]];
    if (session_node)
        headings.push(["websocket sessions", col_x.http, session_node.y - 8]);

    svg.selectAll("text.heading")
        .data(headings)
        .join("text")
        .attr("class", "heading")
        .attr("x", d => d[1]).attr("y", d => d[2])
        .text(d => d[0]);

    link_layer.selectAll("line.link")
        .data(links, d => `${d.source}>${d.target}`)
        .join("line")
        .attr("class", "link");

    const node = node_layer.selectAll("g.node")
        .data(nodes, d => d.id)
        .join(enter => {
            const g = enter.append("g");
            g.append("title");   // the type and its source; see below
            g.append("rect");
            g.append("text").attr("x", 12).attr("y", 25);
            // refcount badge, top-right corner (only where known)
            const b = g.append("g").attr("class", "badge");
            b.append("circle").attr("r", 10);
            b.append("text").attr("text-anchor", "middle").attr("dy", "0.35em");
            return g;
        });

    node.attr("class", d => `node ${d.kind}`)
        .attr("transform", d => `translate(${d.x},${d.y})`);
    node.select(":scope > text").text(d => d.label);

    // source: hover for the type and where it is defined; click to open
    node.each(function (d) {
        const s = source_of(d.type);
        const g = d3.select(this);

        g.classed("linked", !!(s && s.href));
        g.select(":scope > title").text(
            !d.type ? "(no _type_ reported)"
            : !s ? `${d.type}\n(no source location)`
            : `${d.type}\n${s.file}:${s.line}` + (s.href ? "" : "\n(no link provider)"));
        g.on("click", (s && s.href) ? () => window.open(s.href, "_blank") : null);
    });

    // size each box to its label; remember widths for the links
    node.each(function (d) {
        const g = d3.select(this);
        d.h = d.small ? 30 : box_h;
        g.select(":scope > text").attr("y", d.small ? 20 : 25);
        d.w = g.select(":scope > text").node().getComputedTextLength() + 24;
        g.select("rect").attr("width", d.w).attr("height", d.h);

        // refcount: how many rp<> hold this object; red if the snapshot
        // does not account for every hold
        const extra = (d.refcount === undefined) ? 0 : d.refcount - d.expected;
        const badge = g.select("g.badge")
            .attr("display", d.refcount === undefined ? "none" : null)
            .classed("unaccounted", extra !== 0)
            .attr("transform", `translate(${d.w},0)`);
        badge.select("text").text(d.refcount);
        badge.selectAll("title").data([0]).join("title")
            .text(extra === 0
                  ? `refcount ${d.refcount}: every hold shown`
                  : `refcount ${d.refcount}, ${d.expected} shown: ${extra} hold(s) not in this snapshot`);
    });

    // wide enough for the rightmost box (more sessions push the ticker right)
    svg.attr("width", Math.max(900, d3.max(nodes, d => d.x + d.w) + 40));

    // server's facing edge -> endpoint's facing edge
    link_layer.selectAll("line.link")
        .each(function (d) {
            const src = by_id.get(d.source);
            const tgt = by_id.get(d.target);
            const line = d3.select(this);

            if (d.kind === "owns") {
                /* session -> its subscription: down its left side */
                line.attr("x1", src.x + 9).attr("y1", src.y + src.h)
                    .attr("x2", src.x + 9).attr("y2", tgt.y + tgt.h / 2);
            } else if (tgt.y > src.y + box_h) {
                /* a session, below: server's bottom edge -> session's top */
                line.attr("x1", src.x + src.w / 2).attr("y1", src.y + box_h)
                    .attr("x2", tgt.x + tgt.w / 2).attr("y2", tgt.y);
            } else {
                const left = (tgt.x < src.x);   /* http column */

                line.attr("x1", left ? src.x : src.x + src.w)
                    .attr("y1", src.y + box_h / 2)
                    .attr("x2", left ? tgt.x + tgt.w : tgt.x)
                    .attr("y2", tgt.y + box_h / 2);
            }
        });

    // application -> subscription whose sink it holds
    holds_layer.selectAll("path.holds")
        .data(holds, d => d.target)
        .join("path")
        .attr("class", "holds")
        .attr("d", d => {
            const src = by_id.get(d.source);
            const tgt = by_id.get(d.target);
            const x1 = src.x, y1 = src.y + src.h / 2;
            const x2 = tgt.x + tgt.w, y2 = tgt.y + tgt.h / 2;
            const dx = Math.max(40, (x1 - x2) / 2);
            return `M${x1},${y1} C${x1 - dx},${y1} ${x2 + dx},${y2} ${x2},${y2}`;
        });

    // subscription -> the stream endpoint it uses: a curve up and over,
    // into the endpoint's left edge
    uses_layer.selectAll("path.uses")
        .data(uses, d => d.source)
        .join("path")
        .attr("class", "uses")
        .attr("d", d => {
            const src = by_id.get(d.source);
            const tgt = by_id.get(d.target);
            const x1 = src.x + src.w, y1 = src.y + src.h / 2;
            const x2 = tgt.x,         y2 = tgt.y + tgt.h / 2;
            const dx = Math.max(60, (x2 - x1) / 2);
            return `M${x1},${y1} C${x1 + dx},${y1} ${x2 - dx},${y2} ${x2},${y2}`;
        });
}
