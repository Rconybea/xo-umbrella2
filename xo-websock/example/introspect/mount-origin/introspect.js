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
// Context menu (.xo-backlog/xo-websock/issues/13): right-click a box -- or
// focus it (Tab) and press the Menu key / Shift+F10 -- for: open source, show
// its json, copy its type or id.  Only boxes take over right-click; elsewhere
// the browser's menu is untouched (in Firefox, Shift+right-click always gets
// the browser's).
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

// layout: automatic, by ELK (elkjs, its layered algorithm) -- the page builds
// the object graph (boxes and edges) from the snapshot; ELK places the boxes,
// sized to their content, and routes the edges.  Hand-placed columns could
// not grow a box; an expanded box (issue 13) and showing/hiding parts of the
// graph both need this.  .xo-backlog/xo-websock/issues/13, step 2.
const elk = new ELK();

/** the object graph for snapshot @p event: {nodes, edges}.  Edge kinds:
 *  "link"  the server's endpoints and sessions
 *  "owns"  a session's sender and subscriptions
 *  "uses"  a subscription -> the stream endpoint it holds
 *  "holds" the application (ticker) -> a subscription whose sink it holds
 **/
function layout(event) {
    const snap = event.server;
    const ticker = event.ticker;

    const nodes = [];
    const edges = [];
    const edge = (source, target, kind) => edges.push({source, target, kind});

    nodes.push({id: "server", kind: "server",
                label: `Webserver :${snap.listen_port} (${snap.state})`,
                type: snap._type_, obj: snap});

    const endpoint_node = {};   // endpoint object id -> node id

    for (const ep of (snap.endpoints || [])) {
        const id = `${ep.kind}:${ep.stem}`;
        endpoint_node[ep.id] = id;
        nodes.push({id: id, kind: ep.kind, label: ep.pattern, refcount: ep.refcount,
                    type: ep._type_, obj: ep});
        edge("server", id, "link");
    }

    for (const s of (snap.sessions || [])) {
        const id = `session:${s.session_id}`;
        const open = s.sender && s.sender.open;

        nodes.push({id: id, kind: open ? "session" : "session closed",
                    label: `session ${s.session_id}`, type: s._type_, obj: s});
        edge("server", id, "link");

        if (s.sender) {
            const snd = `${id}:sender`;
            nodes.push({id: snd, kind: "sender", label: "sender",
                        refcount: s.sender.refcount, type: s.sender._type_, obj: s.sender,
                        small: true});
            edge(id, snd, "owns");
        }

        for (const sub of (s.subscriptions || [])) {
            const sid = `${id}:sub:${sub.sub_id}`;
            const sink = sub.sink || {};
            // the sink's sender should be this session's: flag it if not
            const astray = s.sender && sink.sender && sink.sender.ref !== s.sender.id;

            nodes.push({id: sid, kind: astray ? "subscription astray" : "subscription",
                        label: `sub ${sub.sub_id} · ${sub.stream}`,
                        refcount: sink.refcount,   // the sink's: slot + its source
                        type: sub._type_, obj: sub, small: true});
            edge(id, sid, "owns");

            // joined BY ID: the endpoint object this subscription holds
            const ep = sub.endpoint && endpoint_node[sub.endpoint.ref];
            if (ep)
                edge(sid, ep, "uses");
        }
    }

    // the application's ticker: a "holds" edge to each subscription whose
    // sink it refers to
    if (ticker) {
        nodes.push({id: "ticker", kind: "app", label: "Ticker (app)",
                    type: ticker._type_, obj: ticker});

        const sub_of_sink = {};
        for (const s of (snap.sessions || []))
            for (const sub of (s.subscriptions || []))
                if (sub.sink) sub_of_sink[sub.sink.id] = `session:${s.session_id}:sub:${sub.sub_id}`;

        for (const r of (ticker.sinks || []))
            if (sub_of_sink[r.ref])
                edge("ticker", sub_of_sink[r.ref], "holds");
    }

    // refcount accounting: the holds this snapshot shows, per object
    const expect = {};   // node id -> expected refcount
    const bump = (k, n) => { expect[k] = (expect[k] || 0) + n; };
    for (const ep of (snap.endpoints || []))
        bump(endpoint_node[ep.id], 1);                       // router's map
    const app_refs = {};
    for (const r of ((ticker && ticker.sinks) || []))
        app_refs[r.ref] = (app_refs[r.ref] || 0) + 1;
    for (const s of (snap.sessions || [])) {
        const sid = `session:${s.session_id}`;
        bump(`${sid}:sender`, 2);                            // record + router
        for (const sub of (s.subscriptions || [])) {
            const subn = `${sid}:sub:${sub.sub_id}`;
            if (sub.endpoint) bump(endpoint_node[sub.endpoint.ref], 1);
            if (sub.sink) {
                bump(subn, 1 + (app_refs[sub.sink.id] || 0)); // slot + app
                if (sub.sink.sender && s.sender && sub.sink.sender.ref === s.sender.id)
                    bump(`${sid}:sender`, 1);
            }
        }
    }
    for (const n of nodes)
        if (n.refcount !== undefined) n.expected = expect[n.id] || 0;

    return {nodes, edges};
}

// a draw is asynchronous (ELK); a newer one supersedes an older one still
// laying out
let draw_seq = 0;

async function draw(event) {
    const seq = ++draw_seq;
    const {nodes, edges} = layout(event);
    const box_h = 40;
    const pad = 20;            // around the whole graph
    const badge_r = 10;        // the refcount badge pokes this far outside a box

    const svg = d3.select("#graph");

    // fixed layers, edges under boxes
    const edge_layer = svg.selectAll("g.edges").data([0]).join("g").attr("class", "edges");
    const node_layer = svg.selectAll("g.nodes").data([0]).join("g").attr("class", "nodes");

    // 1. the boxes, so their text can be measured
    const node = node_layer.selectAll("g.node")
        .data(nodes, d => d.id)
        .join(enter => {
            const g = enter.append("g");
            g.append("title");   // the type and its source; see below
            g.append("rect");
            g.append("text").attr("x", 12).attr("y", 25);
            // refcount badge, top-right corner (only where known)
            const b = g.append("g").attr("class", "badge");
            b.append("circle").attr("r", badge_r);
            b.append("text").attr("text-anchor", "middle").attr("dy", "0.35em");
            return g;
        });

    node.attr("class", d => `node ${d.kind}`);
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

        // focusable, so the keyboard can reach the context menu
        g.attr("tabindex", 0)
         .on("contextmenu", (ev) => {
             ev.preventDefault();

             // from the keyboard the event has no pointer position: use the box
             const r = this.getBoundingClientRect();
             const at_box = (ev.clientX === 0 && ev.clientY === 0);

             show_menu(at_box ? r.left + window.scrollX + 10 : ev.pageX,
                       at_box ? r.bottom + window.scrollY : ev.pageY,
                       d, this);
         });
    });

    // size each box to its label
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

    // 2. ELK: layered, top to bottom; spacing leaves room for the badges
    const graph = {
        id: "root",
        layoutOptions: {
            "elk.algorithm": "layered",
            "elk.direction": "DOWN",
            "elk.edgeRouting": "ORTHOGONAL",
            "elk.spacing.nodeNode": "40",
            "elk.layered.spacing.nodeNodeBetweenLayers": "50",
            "elk.spacing.edgeNode": "20",
        },
        children: nodes.map(d => ({id: d.id, width: d.w, height: d.h})),
        edges: edges.map((e, i) => ({id: `e${i}`, sources: [e.source], targets: [e.target]})),
    };

    const laid = await elk.layout(graph);

    if (seq !== draw_seq)
        return;   // superseded while laying out

    // 3. place the boxes
    const at = new Map(laid.children.map(c => [c.id, c]));
    for (const d of nodes) {
        const c = at.get(d.id);
        d.x = c.x + pad;
        d.y = c.y + pad + badge_r;
    }
    node.attr("transform", d => `translate(${d.x},${d.y})`);

    svg.attr("width", Math.max(900, laid.width + 2 * pad + badge_r))
       .attr("height", laid.height + 2 * pad + badge_r);

    // 4. the edges, as ELK routed them
    const routed = edges.map((e, i) => {
        const le = laid.edges[i];
        const pts = [];
        for (const sec of (le.sections || [])) {
            pts.push(sec.startPoint, ...(sec.bendPoints || []), sec.endPoint);
        }
        return {...e, key: `${e.kind}:${e.source}>${e.target}`, pts};
    });

    edge_layer.selectAll("path.edge")
        .data(routed, d => d.key)
        .join("path")
        .attr("class", d => `edge ${d.kind}`)
        .attr("d", d => d.pts.map((p, k) => `${k ? "L" : "M"}${p.x + pad},${p.y + pad + badge_r}`).join(" "));
}

// ----- context menu -----------------------------------------------------

const menu_el = document.getElementById("ctxmenu");
let menu_owner = null;   // the box element the menu is for, to refocus

/** copy @p text; the clipboard api needs a secure context (https or
 *  localhost), so fall back to a hidden textarea when viewed from another host
 **/
function copy_text(text) {
    if (navigator.clipboard && window.isSecureContext)
        return navigator.clipboard.writeText(text);

    const ta = document.createElement("textarea");
    ta.value = text;
    ta.style.position = "fixed";
    ta.style.opacity = "0";
    document.body.appendChild(ta);
    ta.select();
    document.execCommand("copy");
    ta.remove();
    return Promise.resolve();
}

/** the menu's items for box @p d: [label, action, reason-if-disabled] **/
function menu_items(d) {
    const s = source_of(d.type);
    const no_source = !d.type ? "no _type_ reported"
          : !s ? "no source location"
          : !s.href ? "no link provider" : null;

    return [
        ["Open source", () => window.open(s.href, "_blank"), no_source],
        ["Show JSON", () => show_detail(d), d.obj ? null : "no object"],
        ["Copy type name", () => copy_text(d.type), d.type ? null : "no _type_ reported"],
        ["Copy id", () => copy_text(d.obj.id), (d.obj && d.obj.id) ? null : "no id"],
    ];
}

function show_menu(x, y, d, owner) {
    menu_owner = owner;
    menu_el.replaceChildren();

    const head = document.createElement("div");
    head.className = "ctxhead";
    head.textContent = d.type || d.label;
    menu_el.appendChild(head);

    for (const [label, action, disabled] of menu_items(d)) {
        const b = document.createElement("button");
        b.textContent = label;
        b.disabled = !!disabled;
        if (disabled)
            b.title = disabled;
        b.onclick = () => { hide_menu(); action(); };
        menu_el.appendChild(b);
    }

    menu_el.style.left = `${x}px`;
    menu_el.style.top = `${y}px`;
    menu_el.hidden = false;

    const first = menu_el.querySelector("button:not(:disabled)");
    if (first)
        first.focus();
}

function hide_menu() {
    if (menu_el.hidden)
        return;

    menu_el.hidden = true;

    if (menu_owner)
        menu_owner.focus();
    menu_owner = null;
}

// arrows move between items; Escape closes
menu_el.addEventListener("keydown", (ev) => {
    const items = [...menu_el.querySelectorAll("button:not(:disabled)")];
    const i = items.indexOf(document.activeElement);

    if (ev.key === "Escape") {
        ev.preventDefault();
        hide_menu();
    } else if (ev.key === "ArrowDown" || ev.key === "ArrowUp") {
        ev.preventDefault();
        const n = items.length;
        if (n > 0)
            items[(i + (ev.key === "ArrowDown" ? 1 : n - 1)) % n].focus();
    }
});

// a click outside, resizing or leaving the window closes it.  NOT scrolling:
// the menu sits in page coordinates, so it scrolls along with its box -- and
// closing on scroll closed menus opened while "Show JSON" (or focusing a box)
// scrolled the page
document.addEventListener("mousedown", (ev) => {
    if (!menu_el.hidden && !menu_el.contains(ev.target))
        hide_menu();
});
document.addEventListener("keydown", (ev) => { if (ev.key === "Escape") hide_menu(); });
window.addEventListener("resize", hide_menu);
window.addEventListener("blur", hide_menu);

/** "Show JSON": the box's object alone, below the graph **/
function show_detail(d) {
    const detail_h = document.getElementById("detail-h");
    const detail = document.getElementById("detail");

    detail_h.textContent = `object: ${d.label}` + (d.type ? ` (${d.type})` : "");
    detail.textContent = JSON.stringify(d.obj, null, 2);
    detail_h.hidden = false;
    detail.hidden = false;
    detail_h.scrollIntoView({behavior: "smooth", block: "start"});
}
