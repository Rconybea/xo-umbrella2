// introspect.js -- page side of xo-websock/example/introspect
//
// Protocol (xo-websock, .xo-backlog/xo-websock/issues/04, 06):
//   -> {"cmd": "subscribe", "stream": "/introspect"}
//   <- {"cmd": "subscribed", "stream": "/introspect", "sub_id": N}
//   -> {"cmd": "send", "sub_id": N, "msg": "refresh"}
//   <- {"stream": "/introspect", "sub_id": N, "seq": k, "event": <snapshot>}
//
// Snapshot: {listen_port, state,
//            endpoints: [{kind, stem, pattern}],
//            sessions: [{id, sender_open, n_subscription}]}.

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
        draw(msg.event);
    }
};

function refresh() {
    if (sub_id !== null)
        ws.send(JSON.stringify({cmd: "send", sub_id: sub_id, msg: "refresh"}));
}

refresh_btn.onclick = refresh;

// layout: server in the middle, http endpoints to its left, stream endpoints
// to its right -- so no link passes behind another box.  Sessions in a row
// beneath all of it, linked up to the server
const col_x = {http: 30, server: 330, stream: 640};
const row_h = 58;
const top_y = 40;

function layout(snap) {
    const nodes = [];
    const links = [];
    const n_in = {http: 0, stream: 0};

    for (const ep of (snap.endpoints || [])) {
        const id = `${ep.kind}:${ep.stem}`;
        nodes.push({id: id, kind: ep.kind, label: ep.pattern,
                    x: col_x[ep.kind], y: top_y + row_h * n_in[ep.kind]++});
        links.push({source: "server", target: id});
    }

    // server centred on the taller column
    const n_rows = Math.max(1, n_in.http, n_in.stream);
    nodes.unshift({id: "server", kind: "server",
                   label: `Webserver :${snap.listen_port} (${snap.state})`,
                   x: col_x.server, y: top_y + row_h * (n_rows - 1) / 2});

    // sessions: one row, below the endpoint columns
    const session_y = top_y + row_h * n_rows + 50;
    (snap.sessions || []).forEach((s, i) => {
        const id = `session:${s.id}`;
        const subs = `${s.n_subscription} sub${s.n_subscription === 1 ? "" : "s"}`;
        nodes.push({id: id, kind: s.sender_open ? "session" : "session closed",
                    label: `session ${s.id} · ${subs}`,
                    x: col_x.http + i * 210, y: session_y});
        links.push({source: "server", target: id});
    });

    const n_session = (snap.sessions || []).length;
    const height = (n_session > 0 ? session_y + row_h : top_y + row_h * n_rows) + 20;

    return {nodes, links, height};
}

function draw(snap) {
    const {nodes, links, height} = layout(snap);
    const by_id = new Map(nodes.map(d => [d.id, d]));
    const box_h = 40;

    const svg = d3.select("#graph").attr("height", height);

    // two fixed layers, links under boxes -- new elements go into their
    // layer, so a refresh cannot paint a line over a box
    const link_layer = svg.selectAll("g.links").data([0]).join("g").attr("class", "links");
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
        .data(links, d => d.target)
        .join("line")
        .attr("class", "link");

    const node = node_layer.selectAll("g.node")
        .data(nodes, d => d.id)
        .join(enter => {
            const g = enter.append("g");
            g.append("rect");
            g.append("text").attr("x", 12).attr("y", 25);
            return g;
        });

    node.attr("class", d => `node ${d.kind}`)
        .attr("transform", d => `translate(${d.x},${d.y})`);
    node.select("text").text(d => d.label);

    // size each box to its label; remember widths for the links
    node.each(function (d) {
        const g = d3.select(this);
        d.w = g.select("text").node().getComputedTextLength() + 24;
        g.select("rect").attr("width", d.w).attr("height", box_h);
    });

    // server's facing edge -> endpoint's facing edge
    link_layer.selectAll("line.link")
        .each(function (d) {
            const src = by_id.get(d.source);
            const tgt = by_id.get(d.target);
            const line = d3.select(this);

            if (tgt.y > src.y + box_h) {
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
}
