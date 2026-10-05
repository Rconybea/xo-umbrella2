// introspect.js -- page side of xo-websock/example/introspect
//
// Protocol (xo-websock, .xo-backlog/xo-websock/issues/04, 06):
//   -> {"cmd": "subscribe", "stream": "/introspect"}
//   <- {"cmd": "subscribed", "stream": "/introspect", "sub_id": N}
//   -> {"cmd": "send", "sub_id": N, "msg": "refresh"}
//   <- {"stream": "/introspect", "sub_id": N, "seq": k,
//       "event": {"server": <Webserver json>}}
//
// Webserver json: {_id_, refcount, listen_port, state,
//            endpoints: [{_id_, refcount, kind, stem, pattern, has_receive}],
//            sessions: [{_id_, session_id,
//                        sender: {_id_, refcount, session_id, open},
//                        subscriptions: [{_id_, sub_id, stream,
//                                         endpoint: {_ref_},
//                                         sink: {_id_, refcount, stream, sub_id,
//                                                seq, sender: {_ref_}}}]}]}.
//
// Each object is printed in full once, with "_id_": n; elsewhere as
// {"_ref_": n}.  Ids are numbers, 1, 2, .. within one snapshot
// (.xo-backlog/xo-printjson/issues/02).  The page joins refs to objects by id.
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

/** ids of the boxes whose member rows show their types inline -- name: Type
 *  [metatype] = value -- set from each box's menu ("Show types"); other
 *  boxes show name = value, the type on the name's tooltip and row menu.
 *  Kept across refreshes and collapse/expand, like `expanded`
 **/
const typed_boxes = new Set();

/** the legend is drawn (checkbox "legend"); hidden, it reserves no column
 *  for Fit and the like
 **/
let show_legend = true;

document.getElementById("show-legend").onchange = (ev) => {
    show_legend = ev.target.checked;
    if (last_event)
        draw(last_event);
};

// layout: automatic, by ELK (elkjs, its layered algorithm) -- the page builds
// the object graph (boxes and edges) from the snapshot; ELK places the boxes,
// sized to their content, and routes the edges.  Hand-placed columns could
// not grow a box; an expanded box (issue 13) and showing/hiding parts of the
// graph both need this.  .xo-backlog/xo-websock/issues/13, step 2.
const elk = new ELK();

/** the value of member @p name of reflected struct @p obj, as printjson
 *  writes it: its "_members_" entry (.xo-backlog/xo-printjson/issues/07);
 *  undefined if there is none
 **/
function member_value(obj, name) {
    const m = (obj && obj._members_ || []).find(x => x._name_ === name);
    return m ? m._value_ : undefined;
}

/** the object graph for snapshot @p event: {nodes, edges}.  Edge kinds:
 *  "link"  the server's endpoints and sessions
 *  "owns"  a session's sender and subscriptions; an endpoint's receiver;
 *          a subscription's sink
 *  (a subscription's endpoint is no edge here: its endpoint_ member's
 *  ref edge shows it)
 **/
function layout(event) {
    const snap = member_value(event, "server");   // an IntrospectSnapshot

    const nodes = [];
    const edges = [];
    const edge = (source, target, kind) => edges.push({source, target, kind});

    nodes.push({id: "server", kind: "server",
                label: `Webserver :${snap.listen_port} (${snap.state})`,
                type: snap._canonical_type_, obj: snap});

    for (const ep of (snap.endpoints || [])) {
        const id = `${ep.kind}:${ep.stem}`;
        nodes.push({id: id, kind: ep.kind, label: ep.pattern,
                    type: ep._canonical_type_, obj: ep});
        edge("server", id, "link");

        // its receiver (a stream endpoint's, if any): owned by the endpoint,
        // named by its most-derived type; the receiver_ member refers to it
        if (ep.receiver) {
            const rid = `${id}:receiver`;
            nodes.push({id: rid, kind: "receiver", label: ep.receiver._short_type_ || "receiver",
                        type: ep.receiver._canonical_type_, obj: ep.receiver, small: true});
            edge(id, rid, "owns");
        }
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
                        type: s.sender._canonical_type_, obj: s.sender,
                        small: true});
            edge(id, snd, "owns");
        }

        for (const sub of (s.subscriptions || [])) {
            const sid = `${id}:sub:${sub.sub_id}`;
            const sink = sub.sink || {};
            // the sink's sender should be this session's: flag it if not
            const astray = s.sender && sink.sender && sink.sender._ref_ !== s.sender._id_;

            nodes.push({id: sid, kind: astray ? "subscription astray" : "subscription",
                        label: `sub ${sub.sub_id} · ${sub.stream}`,
                        type: sub._canonical_type_, obj: sub, small: true});
            edge(id, sid, "owns");

            // its sink, owned by the subscription (the router's slot holds
            // it); the subscription's sink_ member refers to it
            if (sub.sink) {
                const kid = `${sid}:sink`;
                nodes.push({id: kid, kind: "sink", label: "sink",
                            type: sub.sink._canonical_type_, obj: sub.sink, small: true});
                edge(sid, kid, "owns");
            }
        }
    }


    // a struct-valued member gets its own box, nested in a group with its
    // holder -- see "nested boxes" below
    for (const d of [...nodes])
        if (d.obj)
            add_nested_boxes(d.id, d.obj, d.id, nodes, edge);

    return {nodes, edges};
}

// ----- expand (issue 13, step 3) -------------------------------------------
//
// A box whose object has "_members_" (the C++ members its printer chose to
// show) toggles open on left-click / Enter: a row per member,
//   name: Type [metatype] = value
// A member whose value is an object with members of its own toggles open in
// place, indented.  A {"_ref_": id} value is an edge from its row to that
// object's box.  Clicking a row's type opens its source.

/** box ids, and member paths ("<box>/<member>/<member>.."), shown open;
 *  kept across refreshes
 **/
const expanded = new Set();

// ----- which boxes are shown (issue 13: showing / hiding parts of the graph) -
//
// The state is a set of WANTED EDGES; the boxes drawn follow from it.
//
// A box's REF EDGES: one per {"_ref_": id} anywhere in its members -- open or
// not -- to the box drawing that object; keyed by the ref's row key
// ("<box>/<member>/..").  A box's CHILDREN are the boxes it owns ("link" /
// "owns": server -> endpoints, sessions; session -> sender, subscriptions;
// endpoint -> receiver; subscription -> sink);
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

// ----- nested boxes (issue 13) ---------------------------------------------
//
// A declared member whose value is a struct with members of its own -- the
// Webserver's url_router_, session_table_, ws_config_; a session's router_
// -- is drawn as a box of its own, not opened in place: its row is a ref
// row (▸ / ▾ (→)) to that box, which is the holder's child (a "nests"
// edge).  The holder and its drawn nested boxes sit inside a group outline
// (an ELK compound node), so they stay together; an edge from elsewhere to
// the nested object arrives at its own box.  A nested box's id is the
// holder's row key for the member, so expanded / wanted keys carry over.
// Declared members only (they carry a _canonical_type_): a struct that is
// an array or map ELEMENT still opens in place.

/** member entry @p m gets a nested box: a declared member, its value a
 *  struct with members (not a ref, an array, a map of refs)
 **/
function nests(m) {
    const v = m._value_;
    return ("_canonical_type_" in m) && has_members(v)
        && !is_ref(v) && !Array.isArray(v) && !is_ref_map(v);
}

/** for box @p owner holding @p obj (row keys under @p path), a nested box
 *  per struct-valued member -- recursively: a nested box's own nested
 *  structs nest in it.  Each joins the group of the top box, @p group
 **/
function add_nested_boxes(owner, obj, group, nodes, edge, path = owner) {
    for (const m of (obj._members_ || [])) {
        if ("_error_" in m || !nests(m))
            continue;
        const v = m._value_;
        const id = `${path}/${m._name_}`;
        nodes.push({id, kind: "nested", label: v._short_type_ || v._name_ || m._name_,
                    type: v._canonical_type_, obj: v, small: true, group});
        edge(owner, id, "nests");
        add_nested_boxes(id, v, group, nodes, edge, id);
    }
}

/** @p v's short type, for display **/
function short_type_of(v) {
    return v._short_type_ || v._name_ || "struct";
}

/** ELK's children for drawn boxes @p nodes, each made by @p elk_node: a
 *  box with drawn nested boxes becomes a group -- a
 *  compound node holding it and them, drawn as an outline
 **/
function elk_children(nodes, elk_node) {
    const in_group = new Map();   // group (its top box) -> [nested box]
    for (const d of nodes)
        if (d.group !== undefined) {
            if (!in_group.has(d.group))
                in_group.set(d.group, []);
            in_group.get(d.group).push(d);
        }

    const out = [];
    for (const d of nodes) {
        if (d.group !== undefined)
            continue;   // inside its group, below
        const nested = in_group.get(d.id);
        if (nested)
            out.push({id: `grp:${d.id}`,
                      layoutOptions: {"elk.padding": "[top=12,left=12,bottom=12,right=12]"},
                      children: [elk_node(d), ...nested.map(elk_node)]});
        else
            out.push(elk_node(d));
    }
    return out;
}

/** the group outlines, @p groups ({id, x, y, w, h}), under everything else
 *  in @p camera: one moving with its boxes, a new one fading in
 **/
function draw_groups(camera, groups) {
    const layer = camera.selectAll(":scope > g.groups").data([0])
          .join(enter => enter.insert("g", ":first-child").attr("class", "groups"));
    layer.selectAll("rect.group")
        .data(groups, g => g.id)
        .join(enter => enter.append("rect").attr("class", "group").attr("rx", 10)
                  .attr("x", g => g.x).attr("y", g => g.y)
                  .attr("width", g => g.w).attr("height", g => g.h)
                  .style("opacity", 0)
                  .call(e => e.transition("fade").delay(t_move).duration(t_show).style("opacity", 1)),
              update => update.call(u => u.transition("move").duration(t_move).ease(d3.easeCubicInOut)
                  .attr("x", g => g.x).attr("y", g => g.y)
                  .attr("width", g => g.w).attr("height", g => g.h)),
              exit => exit.transition("fade").duration(t_fade).style("opacity", 0).remove());
}

/** an ownership edge kind: orders the layers, and makes a child **/
function is_ownership(kind) {
    return kind === "link" || kind === "owns" || kind === "nests";
}

/** the ownership tree of @p edges: {kids: owner -> [child], parent: child -> owner} **/
function ownership(edges) {
    const kids = new Map();
    const parent = new Map();
    for (const e of edges)
        if (is_ownership(e.kind)) {
            if (!kids.has(e.source))
                kids.set(e.source, []);
            kids.get(e.source).push(e.target);
            parent.set(e.target, e.source);
        }
    return {kids, parent};
}

/** how a holder relates to what a ref names, from the ref's declared
 *  (canonical) type -- for an element, its container's: the outermost
 *  smart pointer wins.  "owns": std::unique_ptr; "shares": an intrusive
 *  (rp) or std::shared_ptr; "refers": anything else (T*, T&, unknown).  A
 *  nested struct is "includes" -- by value, not through here
 **/
function ref_kind(type) {
    if (!type)
        return "refers";

    let best = null;
    for (const [pat, kind] of [["std::unique_ptr<", "owns"], ["intrusive_ptr<", "shares"],
                               ["std::shared_ptr<", "shares"]]) {
        const i = type.indexOf(pat);
        if (i >= 0 && (best === null || i < best.i))
            best = {i, kind};
    }
    return best ? best.kind : "refers";
}

/** the strongest of ref kinds @p kinds: includes > owns > shares > refers **/
function strongest_ref_kind(kinds) {
    for (const k of ["includes", "owns", "shares"])
        if (kinds.includes(k))
            return k;
    return "refers";
}

/** every ref in member values @p members, open or not, appended to @p out
 *  as {key, label, ref, ref_kind}: key as member_rows() makes it (under
 *  @p path), label the member path for display (e.g. session_map_["1"]),
 *  ref_kind by ref_kind() from the declared type -- @p type, the
 *  container's, for elements (which declare none)
 **/
function all_refs(members, path, label, out, type = null) {
    for (const m of (members || [])) {
        if ("_error_" in m)
            continue;

        const key = `${path}/${m._name_}`;
        const lab = m._name_.startsWith("[") ? `${label}${m._name_}`
              : label ? `${label}.${m._name_}` : m._name_;
        const v = m._value_;
        const t = m._canonical_type_ || type;

        if (is_ref(v)) {
            out.push({key, label: lab, ref: v._ref_, ref_kind: ref_kind(t)});
        } else if (Array.isArray(v)) {
            all_refs(v.map((x, i) => ({_name_: `[${i}]`, _value_: x})), key, lab, out, t);
        } else if (is_ref_map(v)) {
            all_refs(Object.entries(v).map(([k, x]) => ({_name_: `[${JSON.stringify(k)}]`, _value_: x})),
                     key, lab, out, t);
        } else if (has_members(v)) {
            if (nests(m))   // its own box: refs inside are its
                out.push({key, label: lab, nested: key, ref_kind: "includes"});
            else
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
            const target = r.nested !== undefined ? r.nested : box_of_id.get(r.ref);
            if (target !== undefined && target !== d.id)
                out.push({key: r.key, source: d.id, target, kind: "member", label: r.label,
                          ref_kind: r.ref_kind});
        }
    }
    // an owned box no ref of its owner (or of the owner's nested boxes)
    // reaches: the ownership edge stands in
    const nested = new Set(nodes.filter(d => d.kind === "nested").map(d => d.id));
    const within_ = (s, owner) => s === owner || (nested.has(s) && s.startsWith(owner + "/"));
    for (const e of own_edges)
        if (!out.some(x => within_(x.source, e.source) && x.target === e.target))
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

/** ownership tree @p tree, re-parented through nested boxes: a child its
 *  owner reaches only by a ref from one of the owner's nested boxes (the
 *  server's endpoints, from its url_router_) becomes that nested box's
 *  child, so the tree matches the boxes drawn.  A child the owner refs
 *  directly stays the owner's.  @p edges: showable edges
 **/
function reparent_via_nested(tree, edges) {
    for (const [kid, owner] of [...tree.parent]) {
        if (edges.some(e => e.kind === "member" && e.source === owner && e.target === kid))
            continue;

        const e = edges.find(e => e.kind === "member" && e.target === kid
                             && e.source !== owner && within(e.source, owner));
        if (!e)
            continue;

        tree.parent.set(kid, e.source);
        tree.kids.set(owner, tree.kids.get(owner).filter(k => k !== kid));
        if (!tree.kids.has(e.source))
            tree.kids.set(e.source, []);
        tree.kids.get(e.source).push(kid);
    }
    return tree;
}

/** the key of the edge from owner @p owner to its child @p kid **/
function child_edge(owner, kid) {
    const keys = child_edges(owner, kid);
    return keys.length ? keys[0] : null;
}

/** box @p s is @p owner, or one of its nested boxes **/
function within(s, owner) {
    return s === owner || (nested_ids.has(s) && s.startsWith(owner + "/"));
}

/** keys of the edges that show child @p kid of @p owner: the owner's ref to
 *  it -- or a ref from one of the owner's nested boxes
 *  (e.g. the server's endpoints are reached from its url_router_), with the
 *  edges from the owner down to that nested box.  Else the ownership edge
 **/
function child_edges(owner, kid) {
    const e = showable.find(x => x.target === kid && x.kind === "member" && within(x.source, owner))
          || showable.find(x => x.source === owner && x.target === kid);
    if (!e)
        return [];

    const keys = [e.key];
    for (let x = e.source; x !== owner && tree.parent.has(x); x = tree.parent.get(x)) {
        const up = showable.find(y => y.source === tree.parent.get(x) && y.target === x && y.kind === "member");
        if (!up)
            break;
        keys.push(up.key);
    }
    return keys;
}

/** ids of nested boxes this draw **/
let nested_ids = new Set();

function redraw() {
    if (last_event)
        draw(last_event);
}

/** want the edges down the ownership path from the server to box @p id **/
function show_box(id) {
    for (let x = id; tree.parent.has(x); x = tree.parent.get(x))
        child_edges(tree.parent.get(x), x).forEach(k => wanted.add(k));
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
        if (!shown_box_ids.has(k))
            child_edges(id, k).forEach(key => wanted.add(key));
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
    camera_reset = true;
    if (on)
        showable.filter(e => e.target !== "server").forEach(e => wanted.add(e.key));
    else
        wanted.clear();
    redraw();
}

const row_h = 18;          // a member row
const sub_h = 16;          // a box's title line, below its type
const sub_h_small = 14;    // ... in a small box
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

/** a json object that is a ref: exactly {"_ref_": id} **/
function is_ref(v) {
    return !!v && typeof v === "object" && !Array.isArray(v)
        && Object.keys(v).length === 1 && "_ref_" in v;
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
            if (typeof v._id_ === "number" && "_name_" in v && !box_of_id.has(v._id_))
                box_of_id.set(v._id_, box_id);
            if (Array.isArray(v._members_))
                v._members_.forEach(m => walk(m._value_));
            else if (!("_name_" in v))
                Object.values(v).forEach(walk);
        }
    };
    (members || []).forEach(m => walk(m._value_));
}
const row_pad = 8;         // below the last row

/** true iff canonical type name @p t is a string type, whose values json
 *  prints quoted for good reason: std::basic_string, std::basic_string_view,
 *  char* (const or not), xo's flatstring
 **/
function is_string_type(t) {
    // the type itself, not one taking a string argument (deque<string>)
    return /^(\w+::)*basic_string(_view)?</.test(t)
        || /^(const\s+)?char\s*(const\s*)?\*$/.test(t)
        || /^(\w+::)*flatstring</.test(t);
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
                row.ref = v._ref_;
                // a box it refers to, which can be shown / hidden from here
                const tb = box_of_id.get(v._ref_);
                row.ref_box = (tb !== undefined && tb !== "server") ? tb : null;
                // "(→)" after its ▸ / ▾: this triangle shows another box,
                // not rows in place.  What it refers to is in the tooltip
                val = !box_of_id.has(v._ref_) ? "(→ not drawn)"
                    : row.ref_box ? " (→)" : "(→)";
                row.ref_tip = ref_tooltip(v._ref_);
            } else if (is_ref_map(v)) {
                // a map to objects printed elsewhere: a row per key
                row.expandable = true;
                row.open = expanded.has(key);
                row.tri = row.open ? "▾" : "▸";
                val = ` {${Object.keys(v).length}}`;
            } else if (nests(m)) {
                // its own box (nested boxes): a ref row to it
                row.cls = "ref";
                row.ref_box = key;
                row.ref_tip = `its own box, nested: ${short_type_of(v)}`;
                val = " (→)";
            } else if (typeof v === "object") {
                row.expandable = has_members(v);
                row.open = row.expandable && expanded.has(key);
                // opens in place: just the triangle (its type is on the
                // name's tooltip); one that cannot open shows its name
                if (row.expandable)
                    row.tri = row.open ? "▾" : "▸";
                val = row.expandable ? "" : (v._name_ || "{…}");
            } else {
                // a string whose declared type is not a string type -- an
                // enum, or a printer's summary ("0 queued") -- shows bare:
                // quotes would claim a C++ string that isn't there
                val = (typeof v === "string" && m._canonical_type_ && !is_string_type(m._canonical_type_))
                    ? v : JSON.stringify(v);
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

// ----- the camera: the graph's own viewport -------------------------------
//
// The svg is a fixed window onto the graph (full width, filling the browser
// window below the controls); the rest of the page never moves for its
// sake.  One group, g.camera, holds the drawing and carries a pan / zoom
// transform (d3.zoom): Shift + drag the background to pan, Shift + wheel to
// zoom, "Fit" to see it all.  Without Shift the browser has the events: the
// wheel scrolls the page.
//
// Anchoring: a click inside a box (or on its triangle, or one of its menu
// items) names it the ANCHOR of the redraw that follows; after layout the
// camera moves -- in step with the boxes' own move -- so that box keeps its
// place in the viewport.  Browsers cannot move the mouse pointer, so the
// drawing moves instead.  No anchor (Refresh): the camera stays; Show all /
// Hide all (and the first draw) centre the drawing at 100%, as Center does.

// ----- transitions (issue 13, step 3) ------------------------------------
//
// A redraw animates: leaving boxes, rows and edges fade out (t_fade, while
// ELK lays out); boxes slide to their new places and outlines resize
// (t_move); then arriving boxes, rows and the edges' new routes fade in
// (t_show).  A new draw interrupts the last one's transitions and goes on
// from where things are.  Edges fade rather than morph: old and new routes
// have different bends, and morphing polylines makes spaghetti.

const t_fade = 120;
const t_move = 350;
const t_show = 150;

/** when the latest draw's transitions end (performance.now() ms) **/
let settled_at = 0;

/** a promise that resolves once the latest draw's transitions have ended --
 *  for tests, which would otherwise read positions mid-move
 **/
function settled() {
    return new Promise(function wait(resolve) {
        const left = settled_at - performance.now();
        if (left <= 0 && pending_draws === 0)
            resolve(true);
        else
            setTimeout(() => wait(resolve), Math.max(left, 20));
    });
}

/** draws started and not yet placed (ELK still laying out) **/
let pending_draws = 0;

/** the box the next draw keeps in place; consumed by that draw **/
let pending_anchor = null;
/** box id -> where it was last drawn, and its size (drawing coordinates) **/
let drawn_at = new Map();
/** the next draw centres the drawing again, scale 1 (Show all / Hide all) **/
let camera_reset = true;
/** the first draw: the camera centres the drawing, at once **/
let first_draw = true;
/** the drawing's size, last draw -- for "Fit" **/
let drawing_size = {w: 0, h: 0};

const graph_svg = d3.select("#graph");

/** pan / zoom only with Shift held: otherwise the wheel scrolls the page
 *  and a drag is the browser's.  Dragging on a box is the box's, not a pan
 **/
const zoom = d3.zoom()
      .scaleExtent([0.2, 3])
      .constrain(keep_a_box_in_view)
      .filter((ev) => ev.shiftKey
              && (ev.type === "wheel"
                  || (!ev.button && !(ev.target.closest && ev.target.closest("g.node")))))
      // Shift+wheel is a sideways scroll to browsers: some report it in
      // deltaX, not deltaY -- take whichever moved
      .wheelDelta((ev) => -(ev.deltaY || ev.deltaX) * (ev.deltaMode === 1 ? 0.05 : ev.deltaMode ? 1 : 0.002))
      .on("zoom", (ev) => {
          graph_svg.select(":scope > g.camera").attr("transform", ev.transform);
          show_zoom_level(ev.transform.k);
      });
graph_svg.call(zoom).on("dblclick.zoom", null);
// Shift + press on the background starts a pan -- to the browser it would
// extend any text selection (d3.zoom only stops a new one starting), and
// dragging that toward the window's edge scrolls the page.  Cancel it, and
// drop the selection.  Capture, on the document: d3.zoom's own handler
// stops the event from reaching any other on the svg
document.addEventListener("mousedown", (ev) => {
    const t = ev.target;
    if (ev.shiftKey && t.closest && t.closest("#graph") && !t.closest("g.node")) {
        ev.preventDefault();
        window.getSelection().removeAllRanges();
    }
}, true);

// the hand cursor only while Shift is held: it advertises the gesture
for (const type of ["keydown", "keyup"])
    window.addEventListener(type, (ev) => graph_svg.classed("panning", ev.shiftKey));
window.addEventListener("blur", () => graph_svg.classed("panning", false));

/** d3.zoom's constrain: a pan or zoom may not take every box out of view --
 *  the centre of at least one must stay inside the viewport @p extent.  If
 *  transform @p t would leave none, move it just enough to put the centre
 *  NEAREST the viewport on its edge: the drawing sticks there, and dragging
 *  back moves at once.  Only gestures pass through here, not the page's own
 *  camera moves (anchoring, Fit, reset)
 **/
function keep_a_box_in_view(t, extent) {
    const [[x0, y0], [x1, y1]] = extent;
    let best = null;

    for (const b of drawn_at.values()) {
        const sx = t.applyX(b.x + b.w / 2), sy = t.applyY(b.y + b.h / 2);
        if (sx >= x0 && sx <= x1 && sy >= y0 && sy <= y1)
            return t;   // one in view: fine

        const cx = Math.min(Math.max(sx, x0), x1), cy = Math.min(Math.max(sy, y0), y1);
        const d2 = (sx - cx) ** 2 + (sy - cy) ** 2;
        if (best === null || d2 < best.d2)
            best = {d2, dx: cx - sx, dy: cy - sy};
    }

    // translate() works in drawing units: screen pixels / k
    return best === null ? t : t.translate(best.dx / t.k, best.dy / t.k);
}

/** the svg fills the browser window below the controls **/
function size_view() {
    const top = graph_svg.node().getBoundingClientRect().top + window.scrollY;
    graph_svg.style("height", `${Math.max(240, window.innerHeight - top - 12)}px`);
}
size_view();
window.addEventListener("resize", size_view);

/** box @p d's header line: its short type **/
function box_type_label(d) {
    return (d.obj && d.obj._short_type_) || d.label;
}

/** box @p d's title, the header's second line -- what tells it apart from
 *  other boxes of its type -- or null where the type says it all: a
 *  sender, sink or receiver (one per holder), a nested struct
 **/
function box_subtitle(d) {
    if (d.kind === "server")
        return `:${d.obj.listen_port} (${d.obj.state})`;
    if (["sender", "sink", "receiver", "nested"].includes(d.kind))
        return null;
    return d.label !== box_type_label(d) ? d.label : null;
}

/** box kinds, by colour -- one colour per C++ type; the kinds that share
 *  one (an endpoint is http or stream) share a legend entry.  Order: as
 *  the graph reads, top down.  Nested boxes come after, one entry per type
 **/
const legend_groups = [
    {kinds: ["server"],                                   sample: "server"},
    {kinds: ["http", "stream"],                           sample: "http"},
    {kinds: ["receiver"],                                 sample: "receiver"},
    {kinds: ["session", "session closed"],                sample: "session"},
    {kinds: ["sender"],                                   sample: "sender"},
    {kinds: ["subscription", "subscription astray"],      sample: "subscription"},
    {kinds: ["sink"],                                     sample: "sink"},
];

/** colours for nested boxes, {fill, stroke}: pale fills no other kind uses **/
const nested_palette = [
    {fill: "#e9f5e1", stroke: "#4f8a3a"},   // pale green
    {fill: "#fdebe0", stroke: "#b5653a"},   // peach
    {fill: "#e1f3fa", stroke: "#2b7fa0"},   // pale cyan
    {fill: "#f1f2dc", stroke: "#7d7f2e"},   // olive
    {fill: "#e6e9fb", stroke: "#5560a8"},   // periwinkle
    {fill: "#f6e6f0", stroke: "#a0527f"},   // mauve
    {fill: "#e4f1ec", stroke: "#3f7d68"},   // sage
    {fill: "#f5efe1", stroke: "#8f6f3a"},   // sand
];

/** canonical type -> its nested boxes' colour, assigned as types first
 *  appear and kept for the page's life, so a refresh never repaints
 **/
const nested_colours = new Map();

/** the colour of nested boxes of canonical type @p type **/
function nested_colour(type) {
    if (!nested_colours.has(type))
        nested_colours.set(type, nested_palette[nested_colours.size % nested_palette.length]);
    return nested_colours.get(type);
}

const legend_margin = 8;   // between the viewport's corner and the legend
const legend_pad = 6;      // inside the legend's panel
const legend_row = 18;     // one entry
const swatch_w = 20;
const swatch_h = 12;
const swatch_gap = 8;      // between swatch and type
const legend_gap = 8;      // between the colours and the edge kinds

/** the legend's width, with its margins (0 while hidden): Fit keeps the
 *  drawing clear of it
 **/
let legend_w = 0;
let legend_h = 0;   // ... and height

/** the legend, fixed in the viewport's top-left corner (not panned or
 *  zoomed): for each colour a box in snapshot @p all has (drawn or not --
 *  so it doesn't flicker as boxes show and hide), a swatch and the short
 *  type, top down.  A kind's swatch is styled by the same CSS rules as its
 *  boxes (.swatch.<kind> -- not .node: it is no box); a nested type's
 *  carries that type's colour, as its boxes do
 **/
function draw_legend(all) {
    const entries = [];
    for (const grp of legend_groups) {
        const d = all.nodes.find(n => grp.kinds.includes(n.kind));
        if (d)
            entries.push({key: grp.sample, cls: grp.sample, colour: null,
                          type: (d.obj && d.obj._short_type_) || d.kind, canonical: d.type || ""});
    }
    for (const d of all.nodes)
        if (d.kind === "nested" && !entries.some(x => x.key === `nested:${d.type}`))
            entries.push({key: `nested:${d.type}`, cls: "nested", colour: nested_colour(d.type),
                          type: d.label, canonical: d.type || ""});

    const lg = graph_svg.selectAll(":scope > g.legend").data([0])
          .join(enter => {
              const g = enter.append("g").attr("class", "legend")
                    .attr("transform", `translate(${legend_margin},${legend_margin})`);
              g.append("rect").attr("class", "panel");
              return g;
          })
          .raise();   // over the drawing
    const e = lg.selectAll(":scope > g.entry")
          .data(entries, x => x.key)
          .join(enter => {
              const g = enter.append("g").attr("class", "entry");
              g.append("title");
              g.append("g").append("rect").attr("width", swatch_w).attr("height", swatch_h);
              g.append("text").attr("x", swatch_w + swatch_gap).attr("y", swatch_h - 2);
              return g;
          });
    e.attr("transform", (x, i) => `translate(${legend_pad},${legend_pad + i * legend_row})`);
    e.select(":scope > title").text(x => x.canonical);
    e.select(":scope > g").attr("class", x => `swatch ${x.cls}`);
    e.select(":scope > g > rect")
        .style("fill", x => x.colour ? x.colour.fill : null)
        .style("stroke", x => x.colour ? x.colour.stroke : null);
    e.select(":scope > text").text(x => x.type);

    // below the colours: how an edge leaves its holder, by its marker
    const edge_kinds = entries.length === 0 ? []
          : [{kind: "includes", text: "includes (by value)"},
             {kind: "owns", text: "owns (unique_ptr)"},
             {kind: "shares", text: "shares (rp, shared_ptr)"},
             {kind: "refers", text: "refers (T*, T&)"}];
    const top = legend_pad + entries.length * legend_row + legend_gap;
    const ek = lg.selectAll(":scope > g.edge-entry")
          .data(edge_kinds, x => x.kind)
          .join(enter => {
              const g = enter.append("g").attr("class", "edge-entry");
              g.append("path").attr("d", `M2,${swatch_h / 2} L${swatch_w + 4},${swatch_h / 2}`);
              g.append("text").attr("x", swatch_w + swatch_gap).attr("y", swatch_h - 2);
              return g;
          });
    ek.attr("transform", (x, i) => `translate(${legend_pad},${top + i * legend_row})`);
    ek.select(":scope > path").attr("class", x => `sample from-${x.kind}`);   // not .edge: no edge's data
    ek.select(":scope > text").text(x => x.text);

    const text_w = d3.max([...e.select(":scope > text").nodes(), ...ek.select(":scope > text").nodes()],
                          t => t.getComputedTextLength()) || 0;
    const w = swatch_w + swatch_gap + text_w + 2 * legend_pad;
    const h = edge_kinds.length === 0 ? (entries.length - 1) * legend_row + swatch_h + 2 * legend_pad
          : top + (edge_kinds.length - 1) * legend_row + swatch_h + legend_pad;
    lg.select(":scope > rect.panel").attr("width", w).attr("height", h);
    const shown = show_legend && entries.length > 0;
    lg.attr("display", shown ? null : "none");
    legend_w = shown ? w + 2 * legend_margin : 0;
    legend_h = shown ? h + 2 * legend_margin : 0;
}

/** the magnification, beside the controls (Shift + wheel zooms) **/
function show_zoom_level(k) {
    const el = document.getElementById("zoom-level");
    if (el)
        el.textContent = `zoom ${Math.round(k * 100)}%`;
}

/** the viewport's inner size, {width, height}: inside the svg's border,
 *  where the camera's coordinates start
 **/
function view_size() {
    const el = graph_svg.node();
    return {width: el.clientWidth, height: el.clientHeight};
}

/** the drawing's extent -- what Fit and Center place -- as a sheet under
 *  everything in camera @p camera, size @p size; resized in step with the
 *  boxes, or at once (@p instant)
 **/
function draw_extent(camera, size, instant) {
    const layer = camera.selectAll(":scope > g.extent").data([0])
          .join(enter => enter.insert("g", ":first-child").attr("class", "extent"));   // under the groups
    const r = layer.selectAll(":scope > rect").data([size])
          .join(enter => enter.append("rect").attr("width", size.w).attr("height", size.h));
    (instant ? r : r.transition("size").duration(t_move).ease(d3.easeCubicInOut))
        .attr("width", size.w).attr("height", size.h);
}

/** the camera putting a drawing of size @p size, at zoom @p k, centred in
 *  the viewport -- the legend, over the drawing, not counted
 **/
function centred(size, k) {
    const r = view_size();
    return d3.zoomIdentity.translate(r.width / 2 - k * size.w / 2, r.height / 2 - k * size.h / 2).scale(k);
}

/** "Center": the drawing centred, at the current zoom **/
const center_btn = document.getElementById("center");
if (center_btn) center_btn.onclick = () => {
    graph_svg.transition("move").duration(t_move).ease(d3.easeCubicInOut)
        .call(zoom.transform, centred(drawing_size, d3.zoomTransform(graph_svg.node()).k));
};

/** the camera for "Fit": drawing @p size as large as fits in the viewport
 *  (at most the zoom's maximum), centred -- as Center would, unless that
 *  would overlap the legend.  Then slid clear of it (right, or down) if
 *  the slack allows; else as large as fits right of the legend, or below
 *  it -- whichever is larger -- centred there
 **/
function fit_camera(size) {
    const r = view_size();
    const w = Math.max(1, size.w), h = Math.max(1, size.h);
    const k_max = zoom.scaleExtent()[1];
    // as large as fits in area [x0, x1] x [y0, y1], centred there
    const fit_in = (x0, y0, x1, y1) => {
        const k = Math.min(k_max, (x1 - x0) / w, (y1 - y0) / h);
        return {k, x: x0 + (x1 - x0 - k * w) / 2, y: y0 + (y1 - y0 - k * h) / 2};
    };
    const at = (f) => d3.zoomIdentity.translate(f.x, f.y).scale(f.k);

    const f = fit_in(0, 0, r.width, r.height);
    if (legend_w === 0 || f.x >= legend_w || f.y >= legend_h)
        return at(f);   // clear of the legend
    // slide clear, along whichever axis has the slack
    if (legend_w + f.k * w <= r.width)
        return at({...f, x: legend_w});
    if (legend_h + f.k * h <= r.height)
        return at({...f, y: legend_h});
    const right = fit_in(legend_w, 0, r.width, r.height);
    const below = fit_in(0, legend_h, r.width, r.height);
    return at(right.k >= below.k ? right : below);
}

/** "Fit": the whole drawing as large as fits; see fit_camera() **/
const fit_btn = document.getElementById("fit");
if (fit_btn) fit_btn.onclick = () => {
    graph_svg.transition("move").duration(t_move).ease(d3.easeCubicInOut)
        .call(zoom.transform, fit_camera(drawing_size));
};

for (const type of ["click", "keydown"])   // keydown: Enter / arrows on a focused box
    document.getElementById("graph").addEventListener(type, (ev) => {
        const g = ev.target.closest && ev.target.closest("g.node");
        pending_anchor = g ? g.__data__.id : null;
    }, true);   // capture: before the handler that redraws

async function draw(event) {
    ++pending_draws;
    try {
        await draw_aux(event);
    } finally {
        --pending_draws;
    }
}

async function draw_aux(event) {
    const seq = ++draw_seq;
    const anchor = pending_anchor;
    pending_anchor = null;
    const all = layout(event);
    draw_legend(all);

    // every box -- shown or not -- for joining refs; only shown ones drawn
    box_of_id = new Map(all.nodes.filter(d => d.obj && d.obj._id_ !== undefined)
                        .map(d => [d.obj._id_, d.id]));
    own_box_ids = new Set(box_of_id.keys());
    for (const d of all.nodes)
        if (d.obj)
            note_nested(d.obj._members_, d.id);

    nested_ids = new Set(all.nodes.filter(d => d.kind === "nested").map(d => d.id));
    showable = showable_edges(all.nodes, all.edges.filter(e => is_ownership(e.kind)));
    tree = reparent_via_nested(ownership(all.edges), showable);
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

    const svg = d3.select("#graph");

    // fixed layers, edges under boxes
    const camera = svg.selectAll(":scope > g.camera").data([0]).join("g").attr("class", "camera");
    define_arrowheads(svg);
    const edge_layer = camera.selectAll("g.edges").data([0]).join("g").attr("class", "edges");
    const node_layer = camera.selectAll("g.nodes").data([0]).join("g").attr("class", "nodes");
    svg.select(":scope > g.legend").raise();   // the legend over the drawing

    // 1. the boxes, so their text can be measured
    const node = node_layer.selectAll("g.node")
        .data(nodes, d => d.id)
        .join(enter => {
            // invisible until placed, then faded in (see 3.).  Marked by a
            // property, not a class: the class attribute is rewritten below
            const g = enter.append("g").property("__arriving", true).style("opacity", 0);
            g.append("title");   // the type and its source; see below
            g.append("rect");
            g.append("text").attr("class", "label").attr("x", 12).attr("y", 25);
            g.append("text").attr("class", "sub").attr("x", 12);   // its title, below the type
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
            return g;
        },
        update => update,
        // leaving: fade out, then gone; inert meanwhile
        exit => exit.classed("leaving", true).interrupt("move")
                    .transition("fade").duration(t_fade).style("opacity", 0).remove());
    // one that was leaving and is back: stop its fade
    node.filter(function () { return this.classList.contains("leaving"); })
        .classed("leaving", false).interrupt("fade").style("opacity", null);

    // edges fade out while ELK lays out; the new routes fade in after (4.)
    edge_layer.selectAll(":scope > g.edge-g").interrupt("fade")
        .transition("fade").duration(t_fade).style("opacity", 0);

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
    node.select(":scope > rect")   // a nested box: its type's colour
        .style("fill", d => d.kind === "nested" ? nested_colour(d.type).fill : null)
        .style("stroke", d => d.kind === "nested" ? nested_colour(d.type).stroke : null);
    // the header: the short type, then -- where it says more -- the box's
    // own title (box_subtitle()); menus and tooltips still name a box by
    // its label
    node.select(":scope > text.label").text(d => box_type_label(d));
    node.select(":scope > text.sub").text(d => box_subtitle(d) ?? "");

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

    // the member rows: name = value; with types shown, name: Type [metatype] = value
    node.select(":scope > g.rows").each(function (d) {
        const box_el = this.parentNode;
        const show_types = typed_boxes.has(d.id);
        const rows = d3.select(this).selectAll("text.row")
              .data(d.rows, r => r.key)
              .join(enter => enter.append("text").property("__arriving", true).style("opacity", 0));

        // where each row already on screen was -- its text x, y and its
        // separator's column -- before this draw re-renders it: a row that
        // moves will travel there from here, with its box's outline
        rows.each(function () {
            const meq = this.querySelector(":scope > tspan.meq");
            this.__was = this.__arriving ? null
                : {x: +this.getAttribute("x"), y: +this.getAttribute("y"),
                   col: meq && meq.hasAttribute("x") ? +meq.getAttribute("x") : null};
        });

        rows.attr("class", r => `row ${r.cls}` + (r.expandable ? " expandable" : ""))
            .attr("x", r => 12 + row_indent * r.depth)
            .on("mouseenter", (ev, r) => highlight_ref([r.key], true))
            .on("mouseleave", (ev, r) => highlight_ref([r.key], false));

        rows.each(function (r) {
            const t = d3.select(this);
            t.selectAll("*").remove();

            const typed = !!r.m._canonical_type_;   // an array element has no declared type
            const s = typed ? source_of(r.m._canonical_type_) : null;

            // types replace the value: name: Type [metatype], a ref row
            // keeping its arrow after; an element row has no declared type,
            // so keeps its value
            const as_type = typed && show_types;
            const tip = row_tooltip(r, as_type);

            // the name: hover for its type; ctrl/cmd-click to open its source
            t.append("tspan")
                .attr("class", "mname" + (s && s.href ? " linked" : ""))
                .text(r.m._name_)
                .on("click", (ev) => {
                    if ((ev.ctrlKey || ev.metaKey) && s && s.href) {
                        ev.stopPropagation();
                        window.open(s.href, "_blank");
                    }
                })
                .append("title").text(tip);

            // the separator before the value (or type), its x the row's
            // alignment column
            t.append("tspan").attr("class", "mval meq").text(row_sep);
            // opens in place: its triangle, a button of its own (see tri_buttons)
            if (r.tri)
                t.append("tspan").attr("class", "tri xtoggle").text(r.tri);
            append_ref_toggle(t, r);
            if (as_type) {
                t.append("tspan")
                    .attr("class", "mtype" + (s && s.href ? " linked" : ""))
                    .text((r.tri || r.ref_box ? " " : "") + (r.m._short_type_ || r.m._canonical_type_))
                    .on("click", (ev) => {
                        ev.stopPropagation();
                        if (s && s.href)
                            window.open(s.href, "_blank");
                    })
                    .append("title").text(tip);
                t.append("tspan").attr("class", "mtag").text(` [${r.m._metatype_ || "?"}]`);
            }
            const val = !as_type ? r.val
                  : r.cls === "ref" ? " " + r.val.trimStart() : "";
            if (val !== "") {   // a struct that opens: its triangle is all
                const mv = t.append("tspan").attr("class", "mval").text(val);
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

    // size each box to its header: the type's line (with the menu button
    // and, outside, the children triangle), and the title's below it
    node.each(function (d) {
        const g = d3.select(this);
        const has_sub = box_subtitle(d) !== null;
        const type_h = d.small ? 30 : box_h;                  // the type's line
        const head_h = type_h + (has_sub ? (d.small ? sub_h_small : sub_h) : 0);
        g.select(":scope > text.label").attr("y", d.small ? 20 : 25);
        g.select(":scope > text.sub").attr("y", d.small ? 20 + sub_h_small : 25 + sub_h)
            .attr("display", has_sub ? null : "none");

        const label_w = g.select(":scope > text.label").node().getComputedTextLength();
        const sub_w = has_sub ? g.select(":scope > text.sub").node().getComputedTextLength() : 0;
        g.select(":scope > g.mbtn")
            .attr("transform", `translate(${12 + label_w + mbtn_gap},${(type_h - mbtn_h) / 2})`);
        d.w = Math.max(12 + label_w + mbtn_gap + mbtn_w + 12, 12 + sub_w + 12);
        const texts = g.selectAll(":scope > g.rows > text.row");
        texts.each(function (r, i) {
            d3.select(this).attr("y", head_h + i * row_h + 13);
        });

        // rows align on their separator (row_sep, before the value),
        // stepping in with nesting: a row at depth n puts it at
        // x0 + row_indent * n, x0 the least that clears every row's name.
        // Names are right-justified against it: a row's text starts at its
        // column less the width of what precedes the separator (its name;
        // with types shown, name: Type [metatype])
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
        g.select(":scope > text.kids").attr("y", type_h / 2 + 5);
        tri_buttons(g.select(":scope > g.rows"));
        d.h = head_h + (d.rows.length ? d.rows.length * row_h + row_pad : 0);
        // resize: animated, but for a box not yet on screen
        const animate = (sel) => this.__arriving ? sel
              : sel.transition("size").duration(t_move).ease(d3.easeCubicInOut);
        animate(g.select("rect")).attr("width", d.w).attr("height", d.h);

        // rows already on screen that moved (a nested row opened or closed
        // above them, or the name column widened): back to where they were,
        // then along with the outline -- same duration and easing, so a row
        // inside the old outline and inside the new stays inside throughout.
        // Their triangle squares move with them
        if (!this.__arriving)
            slide_rows(g.select(":scope > g.rows"));

    });

    // a ref member's edge: from its box to the box drawing the referenced
    // object -- its own box, or the box it is printed nested in.  Parallel
    // ones merge: one line per pair of boxes, carrying every member it
    // stands for (e.g. a session's sender_ and router_.sender_)
    const drawn_members = merge_parallel(member_edges(nodes));
    for (const e of drawn_members)
        edges.push({source: e.port, target: e.target, kind: "member",
                    from: e.source, row_keys: e.keys, labels: e.labels, ref_kind: e.ref_kind});
    // a wanted fallback (an owned box no ref reaches): its ownership edge drawn
    const drawn_own = new Set(showable.filter(e => e.kind !== "member" && wanted.has(e.key))
                              .map(e => `${e.source}>${e.target}`));
    for (const e of edges)
        if (is_ownership(e.kind) && drawn_own.has(`${e.source}>${e.target}`))
            e.drawn = true;

    // boxes a member edge arrives at: each gets one entry port
    const member_in = new Set(edges.filter(e => e.kind === "member").map(e => e.target));

    // 2. ELK: layered, top to bottom
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
            // a cycle (a member edge back up the ownership tree -- a sink's
            // sender_, a sender's target_) is broken by reversing the edges
            // that point from a later box to an earlier one in the input
            // order -- and layout() lists parents before children.  So
            // ownership decides the layering by construction: the default
            // (greedy) strategy put session 1 above the Webserver once sinks
            // added sink -> sender back-edges, priority notwithstanding
            "elk.layered.cycleBreaking.strategy": "MODEL_ORDER",
            // nested boxes: groups are compound nodes, and edges cross into
            // them -- laid out together
            "elk.hierarchyHandling": "INCLUDE_CHILDREN",
        },
        children: elk_children(nodes, d => ({
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
        // ownership (link, owns) decides top-to-bottom; member edges
        // follow it.  Without this an expanded box's ref edge could
        // invert the layering -- e.g. a session's router's url_router_ (in
        // the server box) put the session above the server
        edges: edges.map((e, i) => ({
            id: `e${i}`, sources: [e.source],
            targets: [e.kind === "member" ? `${e.target}#in` : e.target],
            layoutOptions: {"elk.layered.priority.direction":
                            is_ownership(e.kind) ? "10" : "0"},
        })),
    };

    const laid = await elk.layout(graph);

    if (seq !== draw_seq)
        return;   // superseded while laying out

    // 3. place the boxes; keep the anchor box where it was on screen.  A
    // group's children come back relative to the group: made absolute here
    const at = new Map();          // box / group id -> {x, y}, drawing coordinates
    const groups = [];             // the group outlines to draw
    for (const c of laid.children) {
        at.set(c.id, {x: c.x, y: c.y});
        if (c.children) {
            groups.push({id: c.id, x: c.x + pad, y: c.y + pad, w: c.width, h: c.height});
            for (const k of c.children)
                at.set(k.id, {x: c.x + k.x, y: c.y + k.y});
        }
    }
    const camera_to = camera_target(anchor, at, pad,
                                    {w: laid.width + 2 * pad, h: laid.height + 2 * pad});
    for (const d of nodes) {
        const c = at.get(d.id);
        d.x = c.x + pad;
        d.y = c.y + pad;
    }
    // boxes already on screen slide there; new ones appear there, then fade in
    const arriving = node.filter(function () { return this.__arriving; });
    const staying = node.filter(function () { return !this.__arriving; });
    arriving.interrupt("move").attr("transform", d => `translate(${d.x},${d.y})`);
    staying.transition("move").duration(t_move).ease(d3.easeCubicInOut)
        .attr("transform", d => `translate(${d.x},${d.y})`);
    arriving.property("__arriving", false)
        .transition("fade").delay(t_move).duration(t_show).style("opacity", 1);
    // rows that appeared: fade in once their box has grown
    node.selectAll(":scope > g.rows > text.row").filter(function () { return this.__arriving; })
        .property("__arriving", false)
        .transition("fade").delay(t_move).duration(t_show).style("opacity", 1);
    // + slack: d3 starts the transitions on its next timer tick
    settled_at = performance.now() + t_move + t_show + 50;
    drawn_at = new Map(nodes.map(d => [d.id, {x: d.x, y: d.y, w: d.w, h: d.h}]));

    // the camera moves in step with the boxes: the anchor stays put.  The
    // first draw: at once -- nothing on screen yet to move from
    if (camera_to && camera_to.instant)
        svg.interrupt("move").call(zoom.transform, camera_to.t);
    else if (camera_to)
        svg.transition("move").duration(t_move).ease(d3.easeCubicInOut)
            .call(zoom.transform, camera_to.t);
    drawing_size = {w: laid.width + 2 * pad, h: laid.height + 2 * pad};

    draw_groups(camera, groups);
    draw_extent(camera, drawing_size, camera_to && camera_to.instant);

    // 4. the edges, as ELK routed them -- in the frame of their `container`
    // (an edge inside one group comes back relative to that group)
    const laid_edge = new Map(laid.edges.map(le => [le.id, le]));
    const routed = edges.map((e, i) => {
        const le = laid_edge.get(`e${i}`);
        const off = (le.container && le.container !== "root" && at.get(le.container)) || {x: 0, y: 0};
        const pts = [];
        for (const sec of (le.sections || [])) {
            pts.push(sec.startPoint, ...(sec.bendPoints || []), sec.endPoint);
        }
        for (const i in pts)
            pts[i] = {x: pts[i].x + off.x, y: pts[i].y + off.y};
        return {...e, key: `${e.kind}:${e.source}>${e.target}`, pts};
    });

    // ownership edges (link, owns) only order the layers: not drawn, but
    // for a wanted fallback
    // each edge a group: its casing, then the edge.  A later edge's casing
    // cuts a gap in the earlier edges it crosses -- a crossing reads as one
    // line passing under another, not as a junction
    const edge_gs = edge_layer.selectAll(":scope > g.edge-g")
        .data(routed.filter(d => !is_ownership(d.kind) || d.drawn), d => d.key)
        .join(enter => {
            const g = enter.append("g").attr("class", "edge-g");
            g.append("path").attr("class", "casing");
            g.append("path");
            return g;
        });
    // the new routes fade in once the boxes have moved
    edge_gs.interrupt("fade").style("opacity", 0)
        .transition("fade").delay(t_move).duration(t_show).style("opacity", 1);
    const route = d => rounded_path(d.pts.map(p => ({x: p.x + pad, y: p.y + pad})), edge_corner_r);
    edge_gs.select(":scope > path.casing").attr("d", route);
    const paths = edge_gs.select(":scope > path:not(.casing)");
    paths
        .attr("class", d => `edge ${d.kind}` + (d.ref_kind ? ` from-${d.ref_kind}` : ""))
        .attr("d", route)
        // a member edge: hovering it lights its row too
        .on("mouseenter", (ev, d) => d.kind === "member" && highlight_ref(d.row_keys, true))
        .on("mouseleave", (ev, d) => d.kind === "member" && highlight_ref(d.row_keys, false))
        // a member edge says which members it stands for: their rows may not be open
        .each(function (d) {
            d3.select(this).selectAll("title").data(d.kind === "member" ? [d] : [])
                .join("title").text(e => `${box_label.get(e.from) || e.from} · ${e.labels.join(", ")}`
                                    + ` (${e.ref_kind})`);
        });
    order_edges(edge_gs);
}

/** arrowhead markers, one per edge colour (index.html picks one per edge
 *  class with marker-end), and the member edges' exit markers: a filled triangle, its tip at the edge's end,
 *  sized in drawing units -- the hot edge's wider stroke doesn't grow it
 **/
const start_scale = 0.75;   // exit markers: drawn at this fraction of their 12x10 viewBox

function define_arrowheads(svg) {
    // a member edge's colour is its ref kind's (index.html: --edge-<kind>)
    const kind_colour = k => `var(--edge-${k})`;
    const ref_kinds = ["includes", "owns", "shares", "refers"];
    const arrows = [["hot", "#e0730b"], ["own", "#999"],
                    ...ref_kinds.map(k => [k, kind_colour(k)])];
    const defs = svg.selectAll(":scope > defs.arrows").data([0]).join("defs").attr("class", "arrows");
    defs.selectAll("marker.arrow").data(arrows, k => k[0])
        .join(enter => enter.append("marker").attr("class", "arrow")
              .attr("id", k => `arrow-${k[0]}`)
              .attr("viewBox", "0 0 10 10").attr("refX", 10).attr("refY", 5)
              .attr("markerWidth", 9).attr("markerHeight", 9)
              .attr("markerUnits", "userSpaceOnUse").attr("orient", "auto")
              .call(m => m.append("path").attr("d", "M0,0 L10,5 L0,10 Z").style("fill", k => k[1])));

    // where a member edge leaves its holder: how the holder relates to the
    // target -- ■ includes (by value), ◆ owns, ○ shares; refers: none.
    // Drawn from the exit point outward, along the edge, in the edge's
    // colour (orange when hot)
    const shapes = [["includes", "M0,1 L8,1 L8,9 L0,9 Z", true],
                    ["owns", "M0,5 L6,1.5 L12,5 L6,8.5 Z", true],
                    ["shares", "M1,5 A4,4 0 1 1 9,5 A4,4 0 1 1 1,5 Z", false]];
    const starts = [];
    for (const [kind, d, filled] of shapes) {
        starts.push({id: `start-${kind}`, d, colour: kind_colour(kind), filled});
        starts.push({id: `start-${kind}-hot`, d, colour: "#e0730b", filled});
    }
    defs.selectAll("marker.start").data(starts, x => x.id)
        .join(enter => enter.append("marker").attr("class", "start")
              .attr("id", x => x.id)
              .attr("viewBox", "0 0 12 10").attr("refX", 0).attr("refY", 5)
              .attr("markerWidth", 12 * start_scale).attr("markerHeight", 10 * start_scale)
              .attr("markerUnits", "userSpaceOnUse").attr("orient", "auto")
              // the outline as wide as the edge's, whatever the scale
              .call(m => m.append("path").attr("d", x => x.d)
                    .style("fill", x => x.filled ? x.colour : "#fff")
                    .style("stroke", x => x.colour).attr("stroke-width", 1.4 / start_scale)));
}

const edge_corner_r = 6;   // an edge's bends: rounded, this radius at most

/** svg path through points @p pts, straight segments, each bend rounded
 *  (radius @p r, less where a segment is short): a turn then reads apart
 *  from a crossing.  The last segment stays straight, for the arrowhead
 **/
function rounded_path(pts, r) {
    if (pts.length === 0)
        return "";

    let d = `M${pts[0].x},${pts[0].y}`;
    for (let i = 1; i < pts.length - 1; i++) {
        const a = pts[i - 1], b = pts[i], c = pts[i + 1];
        const l1 = Math.hypot(b.x - a.x, b.y - a.y), l2 = Math.hypot(c.x - b.x, c.y - b.y);
        const k = Math.min(r, l1 / 2, l2 / 2);
        if (k < 0.5) {
            d += ` L${b.x},${b.y}`;
            continue;
        }
        const p = {x: b.x + (a.x - b.x) * k / l1, y: b.y + (a.y - b.y) * k / l1};
        const q = {x: b.x + (c.x - b.x) * k / l2, y: b.y + (c.y - b.y) * k / l2};
        d += ` L${p.x},${p.y} Q${b.x},${b.y} ${q.x},${q.y}`;
    }
    if (pts.length > 1) {
        const z = pts[pts.length - 1];
        d += ` L${z.x},${z.y}`;
    }
    return d;
}

/** @p member_edges merged per (source box, target box): {source, target,
 *  port, keys, labels, ref_kind -- the strongest}, in order of each pair's
 *  first member
 **/
function merge_parallel(member_edges) {
    const by_pair = new Map();
    for (const e of member_edges) {
        const pair = `${e.source}>${e.target}`;
        if (!by_pair.has(pair))
            by_pair.set(pair, {source: e.source, target: e.target, port: `${pair}#port`,
                               keys: [], labels: [], ref_kinds: []});
        const m = by_pair.get(pair);
        m.keys.push(e.key);
        m.labels.push(e.label);
        m.ref_kinds.push(e.ref_kind);
    }
    for (const m of by_pair.values())
        m.ref_kind = strongest_ref_kind(m.ref_kinds);
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
        .each(function () { if (on) this.parentNode.parentNode.appendChild(this.parentNode); });   // its group on top
    if (!on)
        order_edges(d3.selectAll("g.edges > g.edge-g"));   // back in rank order
}

/** an edge's stacking rank: member edges over the rest, and among them the
 *  strongest ref kind on top -- where edges share a box's entry port, the
 *  shared stretch and its arrowhead show the closest relation
 **/
const edge_rank = {refers: 1, shares: 2, owns: 3, includes: 4};

/** put edge groups @p edge_gs in stacking order, by edge_rank (stable:
 *  equal ranks keep their order)
 **/
function order_edges(edge_gs) {
    edge_gs.sort((a, b) => (edge_rank[a.ref_kind] || 0) - (edge_rank[b.ref_kind] || 0));
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

/** box @p d's menu item to show or hide its member rows' types; disabled
 *  while it draws no member rows (collapsed, say), where the change would
 *  not show
 **/
function types_item(d) {
    const on = typed_boxes.has(d.id);
    return [on ? "Hide types" : "Show types",
            () => {
                if (on)
                    typed_boxes.delete(d.id);
                else
                    typed_boxes.add(d.id);
                if (last_event)
                    draw(last_event);
            },
            (d.rows && d.rows.length) ? null : "no member rows shown"];
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
        types_item(d),
        [`Hide ${d.label}`, () => { hide_box(d.id); redraw(); },
         d.id === "server" ? "the Webserver box is always shown" : null],
        // one entry per child, in ownership order: Hide if drawn, else Show
        ...(d.children || []).map(k => shown_box_ids.has(k)
            ? [`Hide ▸ ${box_label.get(k) || k}`, () => { hide_box(k); redraw(); }, null]
            : [`Show ▸ ${box_label.get(k) || k}`, () => { child_edges(d.id, k).forEach(key => wanted.add(key));
                                                         redraw(); }, null]),
        ["Open source", () => window.open(s.href, "_blank"), no_source],
        ["Show JSON", () => show_detail(d), d.obj ? null : "no object"],
        ["Copy type name", () => copy_text(d.type), d.type ? null : "no _canonical_type_ reported"],
        ["Copy id", () => copy_text(String(d.obj._id_)),
         (d.obj && d.obj._id_ !== undefined) ? null : "no id"],
    ];
}

/** rows group @p rows_g: each row with a recorded old place (`__was`) that
 *  has moved -- y, x, or separator column -- goes back there and transitions
 *  to its new place; so do the squares behind its triangles (which sit just
 *  after the separator, so move by its column's and the row's y's change)
 **/
function slide_rows(rows_g) {
    const g = rows_g.node();
    const ease = d3.easeCubicInOut;

    for (const text of g.querySelectorAll(":scope > text.row")) {
        const was = text.__was;
        text.__was = null;
        if (!was)
            continue;

        const meq = text.querySelector(":scope > tspan.meq");
        const now = {x: +text.getAttribute("x"), y: +text.getAttribute("y"),
                     col: meq && meq.hasAttribute("x") ? +meq.getAttribute("x") : null};
        const dcol = (was.col !== null && now.col !== null) ? now.col - was.col : now.x - was.x;
        const dy = now.y - was.y;
        if (now.x === was.x && dy === 0 && dcol === 0)
            continue;

        d3.select(text).attr("x", was.x).attr("y", was.y)
            .transition("move").duration(t_move).ease(ease)
            .attr("x", now.x).attr("y", now.y);
        if (meq && was.col !== null && now.col !== null)
            d3.select(meq).attr("x", was.col)
                .transition("move").duration(t_move).ease(ease).attr("x", now.col);

        for (const sq of g.querySelectorAll(":scope > rect.tbtn"))
            if (sq.__text === text) {
                const x = +sq.getAttribute("x"), y = +sq.getAttribute("y");
                d3.select(sq).attr("x", x - dcol).attr("y", y - dy)
                    .transition("move").duration(t_move).ease(ease)
                    .attr("x", x).attr("y", y);
            }
    }
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
              .property("__text", ts.parentNode)
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

/** where the camera should go this draw, {t, instant}: the first draw,
 *  the drawing (size @p size) centred in the viewport (as Center) at scale
 *  1, at once; after a reset (Show all / Hide all), the same, animated; for
 *  anchor box @p anchor, laid out at
 *  @p at (ELK positions), moved by as much as the box moved, scaled -- so
 *  on screen (k * x + t) the box stays put; else null, the camera stays
 **/
function camera_target(anchor, at, pad, size) {
    if (first_draw) {
        first_draw = false;
        camera_reset = false;
        return {t: centred(size, 1), instant: true};
    }
    if (camera_reset) {
        camera_reset = false;
        return {t: centred(size, 1)};
    }

    const was = anchor !== null && drawn_at.get(anchor);
    const c = anchor !== null && at.get(anchor);
    if (!was || !c)
        return null;

    const t = d3.zoomTransform(graph_svg.node());
    const dx = was.x - (c.x + pad), dy = was.y - (c.y + pad);
    return {t: d3.zoomIdentity.translate(t.x + t.k * dx, t.y + t.k * dy).scale(t.k)};
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
 *  source location -- what the row does not show inline; with
 *  @p as_type (the row shows its type in place of its value), its value
 *  first -- not for a ref row, which keeps its arrow, or a struct that opens
 **/
function row_tooltip(r, as_type) {
    if (!r.m._canonical_type_)
        return r.m._name_;

    const s = source_of(r.m._canonical_type_);
    return (as_type && r.cls !== "ref" && r.val.trim() ? `= ${r.val.trim()}\n` : "")
        + `${r.m._name_}: ${r.m._short_type_ || r.m._canonical_type_}  [${r.m._metatype_ || "?"}]`
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
        b.onclick = () => {
            // a box's menu item: that box is the redraw's anchor
            const g = menu_owner && menu_owner.closest && menu_owner.closest("g.node");
            hide_menu();
            pending_anchor = g ? g.__data__.id : null;
            action();
        };
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
