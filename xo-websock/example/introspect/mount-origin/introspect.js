// introspect.js -- page side of xo-websock/example/introspect
//
// Protocol (xo-websock, .xo-backlog/xo-websock/issues/04, 06):
//   -> {"cmd": "subscribe", "stream": "/introspect"}
//   <- {"cmd": "subscribed", "stream": "/introspect", "sub_id": N}
//   -> {"cmd": "send", "sub_id": N, "msg": "refresh"}
//   <- {"stream": "/introspect", "sub_id": N, "seq": k, "event": <snapshot>}
//
// Increment 1: the snapshot is {listen_port, state}; drawn as one box.

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

// one box per object; increment 1 has just the server
function draw(snap) {
    const nodes = [
        {id: "server", label: `Webserver :${snap.listen_port} (${snap.state})`, x: 40, y: 50},
    ];

    const svg = d3.select("#graph");

    const node = svg.selectAll("g.node")
        .data(nodes, d => d.id)
        .join(enter => {
            const g = enter.append("g").attr("class", "node");
            g.append("rect");
            g.append("text").attr("x", 14).attr("y", 28);
            return g;
        });

    node.attr("transform", d => `translate(${d.x},${d.y})`);
    node.select("text").text(d => d.label);

    // size each box to its label
    node.each(function () {
        const g = d3.select(this);
        const w = g.select("text").node().getComputedTextLength();
        g.select("rect").attr("width", w + 28).attr("height", 44);
    });
}
