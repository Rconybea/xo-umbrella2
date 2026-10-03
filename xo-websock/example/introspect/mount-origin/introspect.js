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

// ----- expand (issue 13, step 3) -------------------------------------------
//
// A box whose object has "_members_" (the C++ members its printer chose to
// show) toggles open on left-click / Enter: a row per member,
//   name: Type [metatype] = value
// A member whose value is an object with members of its own toggles open in
// place, indented.  A {"ref": id} value is an edge from its row to that
// object's box.  Clicking a row's type opens its source.

/** box ids, and member paths ("<box>/<member>/<member>.."), shown open;
 *  kept across refreshes
 **/
const expanded = new Set();

const row_h = 18;          // a member row
const row_pad = 8;         // below the last row

/** @p t without namespace qualifiers: xo::web::Foo<xo::web::Bar> -> Foo<Bar> **/
function short_type(t) {
    return t ? t.replace(/\b(\w+::)+/g, "") : "?";
}

function has_members(obj) {
    return !!obj && Array.isArray(obj._members_) && obj._members_.length > 0;
}

/** the rows for @p members (at @p depth, under @p path), and the open
 *  members' rows beneath them, appended to @p out
 **/
function member_rows(members, depth, path, out) {
    for (const m of (members || [])) {
        const key = `${path}/${m._name_}`;
        const row = {key, depth, m, cls: "", ref: null, expandable: false, open: false};
        let val;

        if ("_error_" in m) {
            row.cls = "error";
            val = `⚠ ${m._error_}`;
        } else {
            const v = m._value_;

            if (v === null || v === undefined) {
                val = "null";
            } else if (Array.isArray(v)) {
                val = `[${v.length}]`;
            } else if (typeof v === "object" && Object.keys(v).length === 1 && "ref" in v) {
                row.cls = "ref";
                row.ref = v.ref;
                val = "→";
            } else if (typeof v === "object") {
                row.expandable = has_members(v);
                row.open = row.expandable && expanded.has(key);
                val = (row.expandable ? (row.open ? "▾ " : "▸ ") : "") + short_type(v._name_ || "{…}");
            } else {
                val = JSON.stringify(v);
                if (val.length > 40)
                    val = val.slice(0, 39) + "…";
            }
        }

        row.val = val;
        out.push(row);

        if (row.open)
            member_rows(m._value_._members_, depth + 1, key, out);
    }
}

function toggle(key) {
    if (expanded.has(key))
        expanded.delete(key);
    else
        expanded.add(key);

    if (last_event)
        draw(last_event);
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
            g.append("text").attr("class", "label").attr("x", 12).attr("y", 25);
            g.append("g").attr("class", "rows");
            // refcount badge, top-right corner (only where known)
            const b = g.append("g").attr("class", "badge");
            b.append("circle").attr("r", badge_r);
            b.append("text").attr("text-anchor", "middle").attr("dy", "0.35em");
            return g;
        });

    // which boxes can open, and their member rows
    for (const d of nodes) {
        d.expandable = has_members(d.obj);
        d.open = d.expandable && expanded.has(d.id);
        d.rows = [];
        if (d.open)
            member_rows(d.obj._members_, 0, d.id, d.rows);
    }

    node.attr("class", d => `node ${d.kind}` + (d.expandable ? " expandable" : "")
              + (d.open ? " open" : ""));
    node.select(":scope > text.label")
        .text(d => (d.expandable ? (d.open ? "▾ " : "▸ ") : "") + d.label);

    // the member rows: name: Type [metatype] = value
    node.select(":scope > g.rows").each(function (d) {
        const rows = d3.select(this).selectAll("text.row")
              .data(d.rows, r => r.key)
              .join("text");

        rows.attr("class", r => `row ${r.cls}` + (r.expandable ? " expandable" : ""))
            .attr("x", r => 12 + 14 * r.depth);

        rows.each(function (r) {
            const t = d3.select(this);
            t.selectAll("*").remove();

            t.append("tspan").attr("class", "mname").text(`${r.m._name_}: `);

            const s = source_of(r.m._type_);
            t.append("tspan")
                .attr("class", "mtype" + (s && s.href ? " linked" : ""))
                .text(short_type(r.m._type_))
                .on("click", (ev) => {
                    ev.stopPropagation();
                    if (s && s.href)
                        window.open(s.href, "_blank");
                })
                .append("title")
                .text(r.m._type_ + (s ? `\n${s.file}:${s.line}` : "\n(no source location)"));

            t.append("tspan").attr("class", "mtag").text(` [${r.m._metatype_ || "?"}]`);
            t.append("tspan").attr("class", "mval").text(` = ${r.val}`);
        });

        rows.on("click", (ev, r) => {
            ev.stopPropagation();
            if (r.expandable)
                toggle(r.key);
        });
    });

    // source: hover for the type and where it is defined; click to open
    node.each(function (d) {
        const s = source_of(d.type);
        const g = d3.select(this);

        g.select(":scope > title").text(
            !d.type ? "(no _type_ reported)"
            : !s ? `${d.type}\n(no source location)`
            : `${d.type}\n${s.file}:${s.line}` + (s.href ? "" : "\n(no link provider)"));
        // left-click / Enter: open or close its members.  Its source is in
        // the context menu
        g.on("click", d.expandable ? () => toggle(d.id) : null)
         .on("keydown", (ev) => {
             if (ev.key === "Enter" && d.expandable) {
                 ev.preventDefault();
                 toggle(d.id);
             }
         });

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
        const head_h = d.small ? 30 : box_h;
        g.select(":scope > text.label").attr("y", d.small ? 20 : 25);

        d.w = g.select(":scope > text.label").node().getComputedTextLength() + 24;
        g.selectAll(":scope > g.rows > text.row").each(function (r, i) {
            d3.select(this).attr("y", head_h + i * row_h + 13);
            d.w = Math.max(d.w, 12 + 14 * r.depth + this.getComputedTextLength() + 16);
        });

        d.head_h = head_h;
        d.h = head_h + (d.rows.length ? d.rows.length * row_h + row_pad : 0);
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

    // a ref member's edge: from its row to the referenced object's box
    const node_of_obj = new Map(nodes.filter(d => d.obj && d.obj.id).map(d => [d.obj.id, d.id]));
    for (const d of nodes)
        for (const r of d.rows)
            if (r.ref && node_of_obj.has(r.ref))
                edges.push({source: `${r.key}#port`, target: node_of_obj.get(r.ref),
                            kind: "member", from: d.id});

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
        children: nodes.map(d => ({
            id: d.id, width: d.w, height: d.h,
            // a ref member's edge leaves its row, on the box's right side
            layoutOptions: {"elk.portConstraints": "FIXED_POS"},
            ports: d.rows.map((r, i) => ({r, i})).filter(x => x.r.ref && node_of_obj.has(x.r.ref))
                .map(({r, i}) => ({id: `${r.key}#port`, width: 1, height: 1,
                                   x: d.w, y: d.head_h + i * row_h + row_h / 2,
                                   layoutOptions: {"elk.port.side": "EAST"}})),
        })),
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
    const laid_edge = new Map(laid.edges.map(le => [le.id, le]));
    const routed = edges.map((e, i) => {
        const le = laid_edge.get(`e${i}`);
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
        [expanded.has(d.id) && d.expandable ? "Collapse" : "Expand",
         () => toggle(d.id), d.expandable ? null : "no members shown by its printer"],
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
