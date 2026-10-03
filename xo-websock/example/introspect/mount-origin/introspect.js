// introspect.js -- page side of xo-websock/example/introspect
//
// Protocol (xo-websock, .xo-backlog/xo-websock/issues/04, 06):
//   -> {"cmd": "subscribe", "stream": "/introspect"}
//   <- {"cmd": "subscribed", "stream": "/introspect", "sub_id": N}
//   -> {"cmd": "send", "sub_id": N, "msg": "refresh"}
//   <- {"stream": "/introspect", "sub_id": N, "seq": k,
//       "event": {"server": <Webserver json>, "app_holds": [sink id, ..]}}
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
// app_holds: the ids of the sinks the application holds (the example's
// ticker), for refcount accounting only -- not drawn.
//
// Source links (.xo-backlog/xo-websock/issues/12): every object carries
// _canonical_type_, its C++ type's canonical name (and _short_type_, for
// display: xo-reflect's short name).  /dyn/types maps a type -- template
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
// slot + each application hold (app_holds).  More than that is flagged: a
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
document.getElementById("show-all").onclick = () => show_all_children(true);
document.getElementById("hide-all").onclick = () => show_all_children(false);

/** member rows show their types inline -- name: Type [metatype] = value --
 *  else just name = value, the type on the name's tooltip and row menu
 **/
let show_types = false;
document.getElementById("show-types").onchange = (ev) => {
    show_types = ev.target.checked;
    if (last_event)
        draw(last_event);
};

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
 **/
function layout(event) {
    const snap = event.server;

    const nodes = [];
    const edges = [];
    const edge = (source, target, kind) => edges.push({source, target, kind});

    nodes.push({id: "server", kind: "server",
                label: `Webserver :${snap.listen_port} (${snap.state})`,
                type: snap._canonical_type_, obj: snap});

    const endpoint_node = {};   // endpoint object id -> node id

    for (const ep of (snap.endpoints || [])) {
        const id = `${ep.kind}:${ep.stem}`;
        endpoint_node[ep.id] = id;
        nodes.push({id: id, kind: ep.kind, label: ep.pattern, refcount: ep.refcount,
                    type: ep._canonical_type_, obj: ep});
        edge("server", id, "link");
    }

    for (const s of (snap.sessions || [])) {
        const id = `session:${s.session_id}`;
        const open = s.sender && s.sender.open;

        nodes.push({id: id, kind: open ? "session" : "session closed",
                    label: `session ${s.session_id}`, type: s._canonical_type_, obj: s});
        edge("server", id, "link");

        if (s.sender) {
            const snd = `${id}:sender`;
            nodes.push({id: snd, kind: "sender", label: "sender",
                        refcount: s.sender.refcount, type: s.sender._canonical_type_, obj: s.sender,
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
                        type: sub._canonical_type_, obj: sub, small: true});
            edge(id, sid, "owns");

            // joined BY ID: the endpoint object this subscription holds
            const ep = sub.endpoint && endpoint_node[sub.endpoint.ref];
            if (ep)
                edge(sid, ep, "uses");
        }
    }

    // refcount accounting: the holds this snapshot shows, per object
    const expect = {};   // node id -> expected refcount
    const bump = (k, n) => { expect[k] = (expect[k] || 0) + n; };
    for (const ep of (snap.endpoints || []))
        bump(endpoint_node[ep.id], 1);                       // router's map
    const app_refs = {};
    for (const id of (event.app_holds || []))
        app_refs[id] = (app_refs[id] || 0) + 1;
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

// ----- which boxes are shown (issue 13: showing / hiding parts of the graph) -
//
// The state is a set of WANTED EDGES; the boxes drawn follow from it.
//
// A box's REF EDGES: one per {"ref": id} anywhere in its members -- open or
// not -- to the box drawing that object; keyed by the ref's row key
// ("<box>/<member>/..").  A box's CHILDREN are the boxes it owns ("link" /
// "owns": server -> endpoints, sessions; session -> sender, subscriptions);
// the edge to a child is the owner's ref edge to it (e.g. session_map_["1"]
// -> session 1), or failing one, the ownership edge itself, drawn grey.
//
// DRAWN: the Webserver, and every box reachable from it through wanted edges
// -- so a box goes only when no wanted path reaches it.  Every wanted edge
// between drawn boxes is drawn, from its box's bottom edge, whether or not
// its row is open; so is a visible ref row's edge to a box drawn anyway.
// Ownership edges order the layers, and are otherwise not drawn.
//
//   ▸ / ▾ on a ref row    ▾ while its target is drawn: hide the target (as
//                         menu "Hide ▸"); ▸: want this row's edge
//   ▸n / ▾ beside a box   want its edges to all its children (n undrawn) /
//                         hide those children; ArrowRight / ArrowLeft too
//   menu "Show children (+k)"  want its edges to its k children not drawn
//   menu "Hide children (-m)"  hide its m children drawn
//   menu "Show ▸ <child>" want the edge to that child (a child not drawn)
//   menu "Hide ▸ <child>" hide that child, as its own "Hide <label>" (a child drawn)
//   menu "Hide <label>"   unwant every edge into this box
//   collapse a box        changes nothing drawn: its edges still leave it
//                         (from its bottom edge, rows open or not)
//   Show all / Hide all   every edge (but into the Webserver) / none
//
// Wanted edges out of a box no longer drawn are kept: show the box again
// and what hung off it comes back.

/** keys of the wanted edges; kept across refreshes **/
const wanted = new Set();

/** this draw's showable edges, every box's: {key, source, target, kind,
 *  label}; kind "member" (a ref edge) or the ownership kind (a fallback) **/
let showable = [];

/** this draw's ownership tree: {kids, parent} **/
let tree = {kids: new Map(), parent: new Map()};

/** the ownership tree of @p edges: {kids: owner -> [child], parent: child -> owner} **/
function ownership(edges) {
    const kids = new Map();
    const parent = new Map();
    for (const e of edges)
        if (e.kind === "link" || e.kind === "owns") {
            if (!kids.has(e.source))
                kids.set(e.source, []);
            kids.get(e.source).push(e.target);
            parent.set(e.target, e.source);
        }
    return {kids, parent};
}

/** every ref in member values @p members, open or not, appended to @p out
 *  as {key, label, ref}: key as member_rows() makes it (under @p path),
 *  label the member path for display (e.g. session_map_["1"])
 **/
function all_refs(members, path, label, out) {
    for (const m of (members || [])) {
        if ("_error_" in m)
            continue;

        const key = `${path}/${m._name_}`;
        const lab = m._name_.startsWith("[") ? `${label}${m._name_}`
              : label ? `${label}.${m._name_}` : m._name_;
        const v = m._value_;

        if (is_ref(v)) {
            out.push({key, label: lab, ref: v.ref});
        } else if (Array.isArray(v)) {
            all_refs(v.map((x, i) => ({_name_: `[${i}]`, _value_: x})), key, lab, out);
        } else if (is_ref_map(v)) {
            all_refs(Object.entries(v).map(([k, x]) => ({_name_: `[${JSON.stringify(k)}]`, _value_: x})),
                     key, lab, out);
        } else if (has_members(v)) {
            all_refs(v._members_, key, lab, out);
        }
    }
}

/** the showable edges of boxes @p nodes, ownership edges @p own_edges --
 *  needs box_of_id
 **/
function showable_edges(nodes, own_edges) {
    const out = [];
    for (const d of nodes) {
        const refs = [];
        if (d.obj)
            all_refs(d.obj._members_, d.id, "", refs);
        for (const r of refs) {
            const target = box_of_id.get(r.ref);
            if (target !== undefined && target !== d.id)
                out.push({key: r.key, source: d.id, target, kind: "member", label: r.label});
        }
    }
    // an owned box no ref of its owner reaches: the ownership edge stands in
    for (const e of own_edges)
        if (!out.some(x => x.source === e.source && x.target === e.target))
            out.push({key: `own:${e.source}>${e.target}`, source: e.source, target: e.target,
                      kind: e.kind, label: e.kind});
    return out;
}

/** ids of the boxes drawn: the server, and what wanted edges reach from it **/
function drawn_ids() {
    const drawn = new Set(["server"]);
    for (let grew = true; grew; ) {
        grew = false;
        for (const e of showable)
            if (wanted.has(e.key) && drawn.has(e.source) && !drawn.has(e.target)) {
                drawn.add(e.target);
                grew = true;
            }
    }
    return drawn;
}

/** the key of the edge from owner @p owner to its child @p kid **/
function child_edge(owner, kid) {
    const e = showable.find(x => x.source === owner && x.target === kid && x.kind === "member")
          || showable.find(x => x.source === owner && x.target === kid);
    return e ? e.key : null;
}

function redraw() {
    if (last_event)
        draw(last_event);
}

/** want the edges down the ownership path from the server to box @p id **/
function show_box(id) {
    for (let x = id; tree.parent.has(x); x = tree.parent.get(x)) {
        const k = child_edge(tree.parent.get(x), x);
        if (k)
            wanted.add(k);
    }
    redraw();
}

/** unwant every edge into box @p id -- never the Webserver, always drawn **/
function hide_box(id) {
    for (const e of showable)
        if (e.target === id)
            wanted.delete(e.key);
}

/** the triangle beside box @p id: any child undrawn -> want the edges to
 *  them all; else hide them all
 **/
function toggle_children(id) {
    const kids = tree.kids.get(id) || [];

    if (kids.some(k => !shown_box_ids.has(k)))
        show_children(id);
    else
        hide_children(id);
}

/** want the edges from box @p id to its children not drawn **/
function show_children(id) {
    for (const k of (tree.kids.get(id) || []))
        if (!shown_box_ids.has(k)) {
            const key = child_edge(id, k);
            if (key)
                wanted.add(key);
        }
    redraw();
}

/** hide the children of box @p id that are drawn **/
function hide_children(id) {
    for (const k of (tree.kids.get(id) || []))
        if (shown_box_ids.has(k))
            hide_box(k);
    redraw();
}

/** box @p d's menu items for its children as a group: "Show children (+k)"
 *  for k not drawn, "Hide children (-m)" for m drawn -- each only when its
 *  count is non-zero, so a partly shown box has both
 **/
function children_items(d) {
    if (!d.n_children)
        return [["Show children", null, "owns no boxes"]];

    const n_shown = d.n_children - d.n_hidden;
    const items = [];
    if (d.n_hidden > 0)
        items.push([`Show children (+${d.n_hidden})`, () => show_children(d.id), null]);
    if (n_shown > 0)
        items.push([`Hide children (-${n_shown})`, () => hide_children(d.id), null]);
    return items;
}

/** "Show all" / "Hide all".  Not edges into the Webserver: it is always
 *  drawn, so wanting one keeps nothing shown (its ref row has no ▸ / ▾
 *  either); each still draws while its row is open
 **/
function show_all_children(on) {
    if (on)
        showable.filter(e => e.target !== "server").forEach(e => wanted.add(e.key));
    else
        wanted.clear();
    redraw();
}

const row_h = 18;          // a member row
const mbtn_w = 21;         // the menu button after a box's label: square
const mbtn_h = 21;
const mbtn_gap = 8;        // between label and menu button
const tri_button = 16;     // the square behind a row's triangle
const row_indent = 14;     // a nested member row, further right per level
const row_sep = ": ";      // between a row's name and its value
const in_port_x = 24;      // a member edge enters a box this far from its left

/** object id -> the box drawing it this draw: a box's own object, or an
 *  object printed nested inside a box (a member's value).  A ref to any
 *  other object has no edge to draw, and says so
 **/
let box_of_id = new Map();

/** ids of the objects that have a box of their own this draw (as opposed to
 *  objects printed nested inside one)
 **/
let own_box_ids = new Set();

/** ids of the boxes drawn this draw **/
let shown_box_ids = new Set();

/** box id -> its label, every box this draw (for menu entries) **/
let box_label = new Map();

/** a json object that is a ref: exactly {"ref": id} **/
function is_ref(v) {
    return !!v && typeof v === "object" && !Array.isArray(v)
        && Object.keys(v).length === 1 && "ref" in v;
}

/** a json object that is a map of refs: no _name_, every value a ref or
 *  null (JsonMembers::member_ref_map)
 **/
function is_ref_map(v) {
    return !!v && typeof v === "object" && !Array.isArray(v) && !("_name_" in v)
        && !is_ref(v) && Object.keys(v).length > 0
        && Object.values(v).every(x => x === null || is_ref(x));
}

/** record, as drawn by box @p box_id, every object with an id printed
 *  nested in member values @p members -- not overriding an object that has
 *  a box of its own
 **/
function note_nested(members, box_id) {
    const walk = (v) => {
        if (Array.isArray(v)) {
            v.forEach(walk);
        } else if (v && typeof v === "object" && !is_ref(v)) {
            if (typeof v.id === "string" && "_name_" in v && !box_of_id.has(v.id))
                box_of_id.set(v.id, box_id);
            if (Array.isArray(v._members_))
                v._members_.forEach(m => walk(m._value_));
            else if (!("_name_" in v))
                Object.values(v).forEach(walk);
        }
    };
    (members || []).forEach(m => walk(m._value_));
}
const row_pad = 8;         // below the last row

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
                // an array of objects or refs opens into a row per element
                row.expandable = v.length > 0
                    && v.every(x => x === null || (typeof x === "object" && !Array.isArray(x)));
                row.open = row.expandable && expanded.has(key);
                if (row.expandable) {
                    row.tri = row.open ? "▾" : "▸";
                    val = ` [${v.length}]`;
                } else if (v.every(x => x === null || typeof x !== "object")) {
                    // scalars: the contents themselves, cut like a scalar
                    val = JSON.stringify(v);
                    if (val.length > 40)
                        val = val.slice(0, 39) + "…";
                } else {
                    val = `[${v.length}]`;
                }
            } else if (is_ref(v)) {
                row.cls = "ref";
                row.ref = v.ref;
                // a box it refers to, which can be shown / hidden from here
                const tb = box_of_id.get(v.ref);
                row.ref_box = (tb !== undefined && tb !== "server") ? tb : null;
                // "(→)" after its ▸ / ▾: this triangle shows another box,
                // not rows in place.  What it refers to is in the tooltip
                val = !box_of_id.has(v.ref) ? "(→ not drawn)"
                    : row.ref_box ? " (→)" : "(→)";
                row.ref_tip = ref_tooltip(v.ref);
            } else if (is_ref_map(v)) {
                // a map to objects printed elsewhere: a row per key
                row.expandable = true;
                row.open = expanded.has(key);
                row.tri = row.open ? "▾" : "▸";
                val = ` {${Object.keys(v).length}}`;
            } else if (typeof v === "object") {
                row.expandable = has_members(v);
                row.open = row.expandable && expanded.has(key);
                // opens in place: just the triangle (its type is on the
                // name's tooltip); one that cannot open shows its name
                if (row.expandable)
                    row.tri = row.open ? "▾" : "▸";
                val = row.expandable ? "" : (v._name_ || "{…}");
            } else {
                val = JSON.stringify(v);
                if (val.length > 40)
                    val = val.slice(0, 39) + "…";
            }
        }

        row.val = val;
        out.push(row);

        if (row.open) {
            if (Array.isArray(m._value_)) {
                // element rows: no declared type of their own
                member_rows(m._value_.map((x, i) => ({_name_: `[${i}]`, _value_: x})),
                            depth + 1, key, out);
            } else if (is_ref_map(m._value_)) {
                member_rows(Object.entries(m._value_).map(([k, x]) => ({_name_: `[${JSON.stringify(k)}]`, _value_: x})),
                            depth + 1, key, out);
            } else {
                member_rows(m._value_._members_, depth + 1, key, out);
            }
        }
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

/** ref row @p r's target: drawn -> hide it (as menu "Hide ▸ <child>":
 *  unwant every edge into it); not drawn -> want this row's edge.  It
 *  follows the target box, not the row's own edge -- another edge into the
 *  target (a parallel one, e.g. a session's sender_ and router_.sender_, or
 *  another box's) would otherwise keep it drawn while the row said hidden
 **/
function toggle_ref(r) {
    if (shown_box_ids.has(r.ref_box))
        hide_box(r.ref_box);
    else
        wanted.add(r.key);
    redraw();
}

/** on a ref row whose target is a box (not the Webserver): a ▸ / ▾ before
 *  the arrow -- ▾ while the target is drawn -- showing / hiding it
 **/
function append_ref_toggle(t, r) {
    if (!r.ref_box)
        return;

    const on = shown_box_ids.has(r.ref_box);

    t.append("tspan").attr("class", "tri rtoggle").text(on ? "▾" : "▸")
        .on("click", (ev) => {
            ev.stopPropagation();
            toggle_ref(r);
        })
        .append("title").text(on ? "hide the box it refers to"
                              : "show the box it refers to");
}

/** the member edges to draw, of drawn boxes @p nodes: each wanted edge
 *  between drawn boxes; and each visible ref row's edge to a box drawn
 *  anyway.  In each box's member order
 **/
function member_edges(nodes) {
    const visible = new Set();
    for (const d of nodes)
        for (const r of d.rows)
            visible.add(r.key);

    return showable.filter(e => e.kind === "member"
                           && shown_box_ids.has(e.source) && shown_box_ids.has(e.target)
                           && (wanted.has(e.key) || visible.has(e.key)));
}

// a draw is asynchronous (ELK); a newer one supersedes an older one still
// laying out
let draw_seq = 0;

async function draw(event) {
    const seq = ++draw_seq;
    const all = layout(event);
    tree = ownership(all.edges);

    // every box -- shown or not -- for joining refs; only shown ones drawn
    box_of_id = new Map(all.nodes.filter(d => d.obj && d.obj.id).map(d => [d.obj.id, d.id]));
    own_box_ids = new Set(box_of_id.keys());
    for (const d of all.nodes)
        if (d.obj)
            note_nested(d.obj._members_, d.id);

    showable = showable_edges(all.nodes, all.edges.filter(e => e.kind === "link" || e.kind === "owns"));
    shown_box_ids = drawn_ids();

    box_label = new Map(all.nodes.map(d => [d.id, d.label]));
    const nodes = all.nodes.filter(d => shown_box_ids.has(d.id));
    const edges = all.edges.filter(e => shown_box_ids.has(e.source) && shown_box_ids.has(e.target));
    for (const d of nodes) {
        d.children = tree.kids.get(d.id) || [];
        d.n_children = d.children.length;
        d.n_hidden = d.children.filter(k => !shown_box_ids.has(k)).length;
        d.kids_open = (d.n_children > 0 && d.n_hidden === 0);
    }
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
            // menu button, just after the label: left-click opens the box menu
            const mb = g.append("g").attr("class", "mbtn");
            mb.append("rect").attr("width", mbtn_w).attr("height", mbtn_h).attr("rx", 3);
            // three dots, drawn: the "⋯" glyph is tiny in the monospace font
            for (const dx of [-5, 0, 5])
                mb.append("circle").attr("cx", mbtn_w / 2 + dx).attr("cy", mbtn_h / 2).attr("r", 1.6);
            mb.append("title").text("menu");
            g.append("g").attr("class", "rows");
            // children toggle, left of the box (only where it owns any)
            g.append("text").attr("class", "kids").attr("text-anchor", "end").attr("x", -4);
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
    node.select(":scope > text.label").text(d => d.label);

    // the menu button: opens the box menu below it; not a click on the box
    node.select(":scope > g.mbtn").on("click", function (ev, d) {
        ev.stopPropagation();
        const r = this.getBoundingClientRect();
        show_menu(r.left + window.scrollX, r.bottom + window.scrollY + 2,
                  d.type || d.label, menu_items(d), this.parentNode);
    });

    node.select(":scope > text.kids")
        .attr("display", d => d.n_children ? null : "none")
        .text(d => d.kids_open ? "▾" : `▸${d.n_hidden}`)
        .on("click", (ev, d) => {
            ev.stopPropagation();
            toggle_children(d.id);
        })
        .selectAll("title").data(d => [d]).join("title")
        .text(d => d.kids_open ? "hide its children"
              : `show its children (${d.n_hidden} of ${d.n_children} hidden)`);

    // the member rows: name = value; with "types", name: Type [metatype] = value
    node.select(":scope > g.rows").each(function (d) {
        const box_el = this.parentNode;
        const rows = d3.select(this).selectAll("text.row")
              .data(d.rows, r => r.key)
              .join("text");

        rows.attr("class", r => `row ${r.cls}` + (r.expandable ? " expandable" : ""))
            .attr("x", r => 12 + row_indent * r.depth)
            .on("mouseenter", (ev, r) => highlight_ref([r.key], true))
            .on("mouseleave", (ev, r) => highlight_ref([r.key], false));

        rows.each(function (r) {
            const t = d3.select(this);
            t.selectAll("*").remove();

            const typed = !!r.m._canonical_type_;   // an array element has no declared type
            const s = typed ? source_of(r.m._canonical_type_) : null;

            // the name: hover for its type; ctrl/cmd-click to open its source
            t.append("tspan")
                .attr("class", "mname" + (s && s.href ? " linked" : ""))
                .text(typed && show_types ? `${r.m._name_}: ` : r.m._name_)
                .on("click", (ev) => {
                    if ((ev.ctrlKey || ev.metaKey) && s && s.href) {
                        ev.stopPropagation();
                        window.open(s.href, "_blank");
                    }
                })
                .append("title").text(row_tooltip(r));

            if (typed && show_types) {
                t.append("tspan")
                    .attr("class", "mtype" + (s && s.href ? " linked" : ""))
                    .text(r.m._short_type_ || r.m._canonical_type_)
                    .on("click", (ev) => {
                        ev.stopPropagation();
                        if (s && s.href)
                            window.open(s.href, "_blank");
                    })
                    .append("title").text(row_tooltip(r));
                t.append("tspan").attr("class", "mtag").text(` [${r.m._metatype_ || "?"}]`);
            }

            // the separator before the value, its x the row's alignment
            // column: "name: value"; with types, "name: Type [metatype] =
            // value" (a second ":" would be ambiguous there) -- every row
            // of the types view, element rows included, so they match
            t.append("tspan").attr("class", "mval meq").text(show_types ? " = " : row_sep);
            // opens in place: its triangle, a button of its own (see tri_buttons)
            if (r.tri)
                t.append("tspan").attr("class", "tri xtoggle").text(r.tri);
            append_ref_toggle(t, r);
            if (r.val !== "") {   // a struct that opens: its triangle is all
                const mv = t.append("tspan").attr("class", "mval").text(r.val);
                if (r.ref_tip)
                    mv.append("title").text(r.ref_tip);
            }
        });

        rows.on("click", (ev, r) => {
            ev.stopPropagation();
            if (r.expandable && !ev.ctrlKey && !ev.metaKey)
                toggle(r.key);
        });

        // right-click a row: its own menu, not the box's
        rows.on("contextmenu", (ev, r) => {
            ev.preventDefault();
            ev.stopPropagation();
            show_menu(ev.pageX, ev.pageY, row_menu_head(r), row_menu_items(r), box_el);
        });
    });

    // source: hover for the type and where it is defined; click to open
    node.each(function (d) {
        const s = source_of(d.type);
        const g = d3.select(this);

        g.select(":scope > title").text(
            !d.type ? "(no _canonical_type_ reported)"
            : !s ? `${d.type}\n(no source location)`
            : `${d.type}\n${s.file}:${s.line}` + (s.href ? "" : "\n(no link provider)"));
        // left-click / Enter: open or close its members.  Its source is in
        // the context menu
        g.on("click", d.expandable ? () => toggle(d.id) : null)
         .on("keydown", (ev) => {
             if (ev.key === "Enter" && d.expandable) {
                 ev.preventDefault();
                 toggle(d.id);
             } else if (ev.key === "ArrowRight" && d.n_hidden > 0) {
                 ev.preventDefault();
                 toggle_children(d.id);
             } else if (ev.key === "ArrowLeft" && d.kids_open) {
                 ev.preventDefault();
                 toggle_children(d.id);
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
                       d.type || d.label, menu_items(d), this);
         });
    });

    // size each box to its label
    node.each(function (d) {
        const g = d3.select(this);
        const head_h = d.small ? 30 : box_h;
        g.select(":scope > text.label").attr("y", d.small ? 20 : 25);

        const label_w = g.select(":scope > text.label").node().getComputedTextLength();
        g.select(":scope > g.mbtn")
            .attr("transform", `translate(${12 + label_w + mbtn_gap},${(head_h - mbtn_h) / 2})`);
        d.w = 12 + label_w + mbtn_gap + mbtn_w + 12;
        const texts = g.selectAll(":scope > g.rows > text.row");
        texts.each(function (r, i) {
            d3.select(this).attr("y", head_h + i * row_h + 13);
        });

        // rows align on their separator (row_sep, before the value),
        // stepping in with nesting: a row at depth n puts it at
        // x0 + row_indent * n, x0 the least that clears every row's name.
        // Names are right-justified against it: a row's text starts at its
        // column less the width of what precedes the separator (its name;
        // with "types", name: Type [metatype])
        let x0 = null;
        texts.each(function (r) {
            r.eq_natural = eq_x(this);   // where the separator falls, unaligned
            if (r.eq_natural !== null)
                x0 = Math.max(x0 ?? r.eq_natural, r.eq_natural - row_indent * r.depth);
        });
        texts.each(function (r) {
            if (x0 !== null && r.eq_natural !== null) {
                const col = x0 + row_indent * r.depth;
                const lead = r.eq_natural - (12 + row_indent * r.depth);   // name's width
                d3.select(this).attr("x", col - lead);
                d3.select(this).select(":scope > tspan.meq").attr("x", col);
            }
            const b = this.getBBox();
            d.w = Math.max(d.w, b.x + b.width + 16);
        });

        d.head_h = head_h;
        g.select(":scope > text.kids").attr("y", head_h / 2 + 5);
        tri_buttons(g.select(":scope > g.rows"));
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

    // a ref member's edge: from its box to the box drawing the referenced
    // object -- its own box, or the box it is printed nested in.  Parallel
    // ones merge: one line per pair of boxes, carrying every member it
    // stands for (e.g. a session's sender_ and router_.sender_)
    const drawn_members = merge_parallel(member_edges(nodes));
    for (const e of drawn_members)
        edges.push({source: e.port, target: e.target, kind: "member",
                    from: e.source, row_keys: e.keys, labels: e.labels});
    // a wanted fallback (an owned box no ref reaches): its ownership edge drawn
    const drawn_own = new Set(showable.filter(e => e.kind !== "member" && wanted.has(e.key))
                              .map(e => `${e.source}>${e.target}`));
    for (const e of edges)
        if ((e.kind === "link" || e.kind === "owns") && drawn_own.has(`${e.source}>${e.target}`))
            e.drawn = true;

    // boxes a member edge arrives at: each gets one entry port
    const member_in = new Set(edges.filter(e => e.kind === "member").map(e => e.target));

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
            // network simplex, not Brandes-Koepf: BK keeps the narrowest of
            // four candidate placements, and a near-tie between them can flip
            // when one box changes width -- e.g. opening a subscription moved
            // the Webserver box 200px right.  Network simplex keeps boxes put
            "elk.layered.nodePlacement.strategy": "NETWORK_SIMPLEX",
        },
        children: nodes.map(d => ({
            id: d.id, width: d.w, height: d.h,
            // a ref member's edge leaves the box's bottom edge, near its left,
            // in row order; and enters the box it refers to on its top edge,
            // near the left -- so an edge descending from a box's left part
            // need not dogleg left to reach it
            layoutOptions: {"elk.portConstraints": "FIXED_POS"},
            ports: [
                ...drawn_members.filter(e => e.source === d.id)
                    .map((e, k) => ({id: e.port, width: 1, height: 1,
                                     x: 12 + 10 * k, y: d.h,
                                     layoutOptions: {"elk.port.side": "SOUTH"}})),
                ...(member_in.has(d.id)
                    ? [{id: `${d.id}#in`, width: 1, height: 1, x: in_port_x, y: -1,
                        layoutOptions: {"elk.port.side": "NORTH"}}]
                    : []),
            ],
        })),
        // ownership (link, owns) decides top-to-bottom; uses / member
        // edges follow it.  Without this an expanded box's ref edge could
        // invert the layering -- e.g. a session's router's url_router_ (in
        // the server box) put the session above the server
        edges: edges.map((e, i) => ({
            id: `e${i}`, sources: [e.source],
            targets: [e.kind === "member" ? `${e.target}#in` : e.target],
            layoutOptions: {"elk.layered.priority.direction":
                            (e.kind === "link" || e.kind === "owns") ? "10" : "0"},
        })),
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

    // ownership edges (link, owns) only order the layers: not drawn, but
    // for a wanted fallback
    edge_layer.selectAll("path.edge")
        .data(routed.filter(d => (d.kind !== "link" && d.kind !== "owns") || d.drawn), d => d.key)
        .join("path")
        .attr("class", d => `edge ${d.kind}`)
        .attr("d", d => d.pts.map((p, k) => `${k ? "L" : "M"}${p.x + pad},${p.y + pad + badge_r}`).join(" "))
        // a member edge: hovering it lights its row too
        .on("mouseenter", (ev, d) => d.kind === "member" && highlight_ref(d.row_keys, true))
        .on("mouseleave", (ev, d) => d.kind === "member" && highlight_ref(d.row_keys, false))
        // a member edge says which members it stands for: their rows may not be open
        .each(function (d) {
            d3.select(this).selectAll("title").data(d.kind === "member" ? [d] : [])
                .join("title").text(e => `${box_label.get(e.from) || e.from} · ${e.labels.join(", ")}`);
        });
}

/** @p member_edges merged per (source box, target box): {source, target,
 *  port, keys, labels}, in order of each pair's first member
 **/
function merge_parallel(member_edges) {
    const by_pair = new Map();
    for (const e of member_edges) {
        const pair = `${e.source}>${e.target}`;
        if (!by_pair.has(pair))
            by_pair.set(pair, {source: e.source, target: e.target, port: `${pair}#port`,
                               keys: [], labels: []});
        const m = by_pair.get(pair);
        m.keys.push(e.key);
        m.labels.push(e.label);
    }
    return [...by_pair.values()];
}

/** light up (@p on) or restore the ref rows with keys @p row_keys and the
 *  member edges standing for them.  Member edges leave the bottom of the
 *  box, so this is how a row and its edge are seen to belong together:
 *  hovering a row lights its edge; hovering an edge, every row it stands for
 **/
function highlight_ref(row_keys, on) {
    d3.selectAll("text.row").filter(r => r && row_keys.includes(r.key)).classed("hot", on);
    d3.selectAll("path.edge.member").filter(e => e && e.row_keys.some(k => row_keys.includes(k)))
        .classed("hot", on)
        .each(function () { if (on) this.parentNode.appendChild(this); });   // on top
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
    const no_source = !d.type ? "no _canonical_type_ reported"
          : !s ? "no source location"
          : !s.href ? "no link provider" : null;

    return [
        [expanded.has(d.id) && d.expandable ? "Collapse" : "Expand",
         () => toggle(d.id), d.expandable ? null : "no members shown by its printer"],
        ...children_items(d),
        [`Hide ${d.label}`, () => { hide_box(d.id); redraw(); },
         d.id === "server" ? "the Webserver box is always shown" : null],
        // one entry per child, in ownership order: Hide if drawn, else Show
        ...(d.children || []).map(k => shown_box_ids.has(k)
            ? [`Hide ▸ ${box_label.get(k) || k}`, () => { hide_box(k); redraw(); }, null]
            : [`Show ▸ ${box_label.get(k) || k}`, () => { const key = child_edge(d.id, k);
                                                         if (key) wanted.add(key); redraw(); }, null]),
        ["Open source", () => window.open(s.href, "_blank"), no_source],
        ["Show JSON", () => show_detail(d), d.obj ? null : "no object"],
        ["Copy type name", () => copy_text(d.type), d.type ? null : "no _canonical_type_ reported"],
        ["Copy id", () => copy_text(d.obj.id), (d.obj && d.obj.id) ? null : "no id"],
    ];
}

/** behind each triangle in rows group @p rows_g, a rounded square: plain
 *  until hovered, then white, part-transparent -- as the menu button.
 *  Clicking it does what clicking its triangle does.  SVG text takes no
 *  background, so the square is a rect placed by the triangle's measured box
 **/
function tri_buttons(rows_g) {
    const g = rows_g.node();
    rows_g.selectAll(":scope > rect.tbtn").remove();

    for (const ts of g.querySelectorAll(":scope > text.row > tspan.tri")) {
        const b = ts.getBBox();
        const z = tri_button;
        const rect = d3.select(g).insert("rect", ":first-child")
              .attr("class", "tbtn").attr("rx", 3)
              .attr("x", b.x + b.width / 2 - z / 2).attr("y", b.y + b.height / 2 - z / 2)
              .attr("width", z).attr("height", z);
        const hot = (on) => rect.classed("hot", on);

        d3.select(ts).on("mouseenter.tbtn", () => hot(true)).on("mouseleave.tbtn", () => hot(false));
        rect.on("mouseenter", () => hot(true))
            .on("mouseleave", () => hot(false))
            .on("click", (ev) => {
                ev.stopPropagation();
                ts.dispatchEvent(new MouseEvent("click", {bubbles: true}));
            });
    }
}

/** the x at which row text @p text's separator (before the value) starts, as laid out without
 *  alignment; null if it has none
 **/
function eq_x(text) {
    let n = 0;   // characters before the separator
    for (const ts of text.querySelectorAll(":scope > tspan")) {
        if (ts.classList.contains("meq"))
            return text.getStartPositionOfChar(n).x;
        // its own text, not a <title> tooltip's
        for (const c of ts.childNodes)
            if (c.nodeType === Node.TEXT_NODE)
                n += c.nodeValue.length;
    }
    return null;
}

/** what object @p id (a ref's target) is, for its row's tooltip: the box
 *  drawing it, or the box it is printed inside; drawn or not
 **/
function ref_tooltip(id) {
    const b = box_of_id.get(id);
    if (b === undefined)
        return `refers to ${id}: no box draws it`;

    const where = own_box_ids.has(id) ? `refers to ${box_label.get(b) || b}`
          : `refers to an object printed inside ${box_label.get(b) || b}`;
    return `${where}${shown_box_ids.has(b) ? "" : " (hidden)"}\n${id}`;
}

/** member row @p r's tooltip: its type, metatype, canonical type and
 *  source location -- what the row no longer shows inline
 **/
function row_tooltip(r) {
    if (!r.m._canonical_type_)
        return r.m._name_;

    const s = source_of(r.m._canonical_type_);
    return `${r.m._name_}: ${r.m._short_type_ || r.m._canonical_type_}  [${r.m._metatype_ || "?"}]`
        + `\n${r.m._canonical_type_}`
        + (s ? `\n${s.file}:${s.line}` : "\n(no source location)")
        + (s && s.href ? "\nctrl-click: open source" : "");
}

/** member row @p r's menu heading: name: Type **/
function row_menu_head(r) {
    return r.m._canonical_type_ ? `${r.m._name_}: ${r.m._short_type_ || r.m._canonical_type_}`
        : r.m._name_;
}

/** member row @p r's menu items: [label, action, reason-if-disabled] **/
function row_menu_items(r) {
    const t = r.m._canonical_type_;
    const s = source_of(t);
    const no_source = !t ? "no declared type"
          : !s ? "no source location"
          : !s.href ? "no link provider" : null;

    const items = [
        ["Open source", () => window.open(s.href, "_blank"), no_source],
        ["Copy type name", () => copy_text(t), t ? null : "no declared type"],
    ];
    if (r.expandable)
        items.push([r.open ? "Collapse" : "Expand", () => toggle(r.key), null]);
    if (r.ref_box) {
        const label = box_label.get(r.ref_box) || r.ref_box;
        items.push(shown_box_ids.has(r.ref_box)
                   ? [`Hide ▸ ${label}`, () => toggle_ref(r), null]
                   : [`Show ▸ ${label}`, () => toggle_ref(r), null]);
    }
    return items;
}

/** open the context menu at page position @p x, @p y: heading @p head,
 *  items @p items ([label, action, reason-if-disabled]); focus returns to
 *  @p owner on close
 **/
function show_menu(x, y, head_text, items, owner) {
    menu_owner = owner;
    menu_el.replaceChildren();

    const head = document.createElement("div");
    head.className = "ctxhead";
    head.textContent = head_text;
    menu_el.appendChild(head);

    for (const [label, action, disabled] of items) {
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
