// introspect.js -- page side of xo-websock/example/introspect
//
// Protocol (xo-websock, .xo-backlog/xo-websock/issues/04, 06):
//   -> {"cmd": "subscribe", "stream": "/introspect"}
//   <- {"cmd": "subscribed", "stream": "/introspect", "sub_id": N}
//   -> {"cmd": "send", "sub_id": N, "msg": "refresh"}
//   <- {"stream": "/introspect", "sub_id": N, "seq": k, "event": <snapshot>}
//
// Snapshot: {listen_port, state, endpoints: [{kind, stem, pattern}]}.

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
// to its right -- so no link passes behind another box
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

    return {nodes, links, height: top_y + row_h * n_rows + 20};
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

    // column headings
    svg.selectAll("text.heading")
        .data([["http", col_x.http], ["stream", col_x.stream]])
        .join("text")
        .attr("class", "heading")
        .attr("x", d => d[1]).attr("y", 22)
        .text(d => `${d[0]} endpoints`);

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
            const left = (tgt.x < src.x);   /* http column */

            d3.select(this)
                .attr("x1", left ? src.x : src.x + src.w)
                .attr("y1", src.y + box_h / 2)
                .attr("x2", left ? tgt.x + tgt.w : tgt.x)
                .attr("y2", tgt.y + box_h / 2);
        });
}
