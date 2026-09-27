// introspect.js -- page side of xo-websock/example/introspect
//
// Protocol (xo-websock, .xo-backlog/xo-websock/issues/04, 06):
//   -> {"cmd": "subscribe", "stream": "/introspect"}
//   <- {"cmd": "subscribed", "stream": "/introspect", "sub_id": N}
//   -> {"cmd": "send", "sub_id": N, "msg": "refresh"}
//   <- {"stream": "/introspect", "sub_id": N, "seq": k,
//       "event": {"server": <Webserver json>}}
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

"use strict";

const status_el = document.getElementById("status");
const raw_el = document.getElementById("raw");
const refresh_btn = document.getElementById("refresh");

let sub_id = null;

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
        refresh();
    } else if (msg.error) {
        status_el.textContent = `error: ${msg.error}`;
    } else if ("event" in msg) {
        raw_el.textContent = JSON.stringify(msg, null, 2);
        draw(msg.event.server);
    }
};

function refresh() {
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

function layout(snap) {
    const nodes = [];
    const links = [];
    const n_in = {http: 0, stream: 0};

    const endpoint_node = {};   // endpoint object id -> node id

    for (const ep of (snap.endpoints || [])) {
        const id = `${ep.kind}:${ep.stem}`;
        endpoint_node[ep.id] = id;
        nodes.push({id: id, kind: ep.kind, label: ep.pattern, refcount: ep.refcount,
                    x: col_x[ep.kind], y: top_y + row_h * n_in[ep.kind]++});
        links.push({source: "server", target: id});
    }

    // server centred on the taller column
    const n_rows = Math.max(1, n_in.http, n_in.stream);
    nodes.unshift({id: "server", kind: "server",
                   label: `Webserver :${snap.listen_port} (${snap.state})`,
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
                    label: `session ${s.session_id}`, x: x, y: session_y});
        links.push({source: "server", target: id});

        // the session's sender: first under it, with its refcount
        if (s.sender) {
            const snd = `${id}:sender`;
            nodes.push({id: snd, kind: "sender", label: "sender",
                        refcount: s.sender.refcount,
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
                        x: x + 18, y: session_y + row_h + (k + 1) * sub_h, small: true});
            links.push({source: id, target: sid, kind: "owns"});

            // joined BY ID: the endpoint object this subscription holds
            const ep = sub.endpoint && endpoint_node[sub.endpoint.ref];
            if (ep)
                uses.push({source: sid, target: ep});
        });

        n_sub_max = Math.max(n_sub_max, subs.length + 1);   // + the sender
    });

    const n_session = (snap.sessions || []).length;
    const height = (n_session > 0
                    ? session_y + row_h + n_sub_max * sub_h
                    : top_y + row_h * n_rows) + 20;

    return {nodes, links, uses, height};
}

function draw(snap) {
    const {nodes, links, uses, height} = layout(snap);
    const by_id = new Map(nodes.map(d => [d.id, d]));
    const box_h = 40;

    const svg = d3.select("#graph").attr("height", height);

    // two fixed layers, links under boxes -- new elements go into their
    // layer, so a refresh cannot paint a line over a box
    const link_layer = svg.selectAll("g.links").data([0]).join("g").attr("class", "links");
    const uses_layer = svg.selectAll("g.uses").data([0]).join("g").attr("class", "uses");
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

    // size each box to its label; remember widths for the links
    node.each(function (d) {
        const g = d3.select(this);
        d.h = d.small ? 30 : box_h;
        g.select(":scope > text").attr("y", d.small ? 20 : 25);
        d.w = g.select(":scope > text").node().getComputedTextLength() + 24;
        g.select("rect").attr("width", d.w).attr("height", d.h);

        // refcount: how many rp<> hold this object
        const badge = g.select("g.badge")
            .attr("display", d.refcount === undefined ? "none" : null)
            .attr("transform", `translate(${d.w},0)`);
        badge.select("text").text(d.refcount);
        badge.append("title").text(`refcount ${d.refcount}`);
    });

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
