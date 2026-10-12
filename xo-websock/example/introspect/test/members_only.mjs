// the page reads "_members_" alone (.xo-backlog/xo-websock/issues/17): take
// the live snapshot, drop every view-model key (endpoints, sessions,
// listen_port, state, refcount, ..) and place each object at its first ref
// in member order -- the shape printjson's first-encounter placement gives
// once websock's printers stop writing those lists (issues/15, 16).  The
// page must draw it exactly as it draws the live snapshot: same boxes,
// labels, edges, rows.
import fs from "node:fs";
const [,, cdp_port, port, out] = process.argv;
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
// two more sessions: one subscribed (a subscription and its sink), one not
const cl1 = new WebSocket(`ws://localhost:${port}/`, "lws-minimal");
await new Promise(r => cl1.onopen = r); cl1.send('{"cmd":"subscribe","stream":"/demo/1"}');
const cl2 = new WebSocket(`ws://localhost:${port}/`, "lws-minimal");
await new Promise(r => cl2.onopen = r);
await sleep(300);
const tgt = await (await fetch(`http://localhost:${cdp_port}/json/new?http://localhost:${port}/`, {method: "PUT"})).json();
const ws = new WebSocket(tgt.webSocketDebuggerUrl); await new Promise(r => ws.onopen = r);
let seq = 0; const pending = new Map();
ws.onmessage = (ev) => { const m = JSON.parse(ev.data); if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); } };
const call = (method, params = {}) => new Promise(r => { const id = ++seq; pending.set(id, r); ws.send(JSON.stringify({id, method, params})); });
const ev = async (expr) => { const r = await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true}); if (r.result.exceptionDetails) console.log("EXC", JSON.stringify(r.result.exceptionDetails).slice(0, 300)); return r.result.result?.value; };
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok  " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1500, height: 1000, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`typeof last_event !== "undefined" && !!last_event && document.querySelectorAll("g.node").length >= 1`)); i++) await sleep(100);
await sleep(500);

// the members-only snapshot, built in the page: __orig (live), __mo (moved)
await ev(`(() => {
  const members_only = (e0) => {
    const e = structuredClone(e0);
    const ids = objects_by_id(e);
    for (const o of ids.values())
      for (const k of Object.keys(o))
        if (!k.startsWith("_")) delete o[k];
    const placed = new Set();
    const visit = (o) => { placed.add(o._id_); fix_members(o); };
    const fix_members = (o) => (o._members_ || []).forEach(m => { if ("_value_" in m) m._value_ = val(m._value_); });
    const val = (v) => {
      if (Array.isArray(v)) return v.map(val);
      if (!v || typeof v !== "object") return v;
      if (is_ref(v)) {
        const o = ids.get(v._ref_);
        if (o && !placed.has(v._ref_)) { visit(o); return o; }
        return v;
      }
      if (typeof v._id_ === "number") {
        if (placed.has(v._id_)) return {_ref_: v._id_};
        visit(v); return v;
      }
      if (Array.isArray(v._members_)) { fix_members(v); return v; }
      if (!("_name_" in v)) for (const k of Object.keys(v)) v[k] = val(v[k]);   // a map
      return v;
    };
    visit(member_value(e, "server"));
    return e;
  };
  window.__orig = structuredClone(last_event);
  window.__mo = members_only(last_event);
  return 1; })()`);

// the moved snapshot: every object placed in members, once; refs resolve
const shape = await ev(`(() => {
  const n_ids = (e) => { let n = 0; const w = (v) => { if (Array.isArray(v)) v.forEach(w);
      else if (v && typeof v === "object") { if (typeof v._id_ === "number" && !("_ref_" in v)) ++n; Object.values(v).forEach(w); } };
    w(e); return n; };
  const refs = []; const w = (v) => { if (Array.isArray(v)) v.forEach(w);
      else if (v && typeof v === "object") { if (is_ref(v)) refs.push(v._ref_); else Object.values(v).forEach(w); } };
  w(__mo._members_);
  const placed = objects_by_id(__mo), unplaced = new Set((__mo._unplaced_ || []).map(x => x._ref_));
  const vm = [...placed.values()].filter(o => Object.keys(o).some(k => !k.startsWith("_"))).length;
  const srv = member_value(__mo, "server");
  const smap = member_value(member_value(srv, "session_table"), "session_map_");
  return {orig: objects_by_id(__orig).size, placed: placed.size, ids_written: n_ids(__mo), vm,
          dangling: refs.filter(r => !placed.has(r) && !unplaced.has(r)),
          sessions_in_map: Object.values(smap).filter(v => typeof v._id_ === "number").length,
          n_sessions: Object.keys(smap).length,
          srv_keys: Object.keys(srv).filter(k => !k.startsWith("_"))}; })()`);
console.log("   shape:", JSON.stringify(shape));
check(shape.vm === 0 && shape.srv_keys.length === 0, "no view-model keys left (server keys: " + JSON.stringify(shape.srv_keys) + ")");
check(shape.placed === shape.orig, `every object placed in members: ${shape.placed} of ${shape.orig}`);
check(shape.ids_written === shape.placed, `each written once: ${shape.ids_written} _id_s for ${shape.placed} objects`);
check(shape.dangling.length === 0, "every ref resolves: dangling " + JSON.stringify(shape.dangling));
check(shape.n_sessions >= 3 && shape.sessions_in_map === shape.n_sessions,
      `sessions placed in session_map_: ${shape.sessions_in_map} of ${shape.n_sessions}`);

// layout: same boxes (id, kind, label, type) and edges
const lay = await ev(`(() => {
  const f = (e) => { const g = layout(e);
    return JSON.stringify({nodes: g.nodes.map(d => [d.id, d.kind, d.label, d.type]),
                           edges: g.edges.map(x => [x.source, x.target, x.kind])}); };
  const a = f(__orig), b = f(__mo);
  return {same: a === b, n: JSON.parse(a).nodes.length, a: a.slice(0, 400), b: b.slice(0, 400)}; })()`);
check(lay.same && lay.n >= 10, `layout(): the same ${lay.n} boxes and edges` + (lay.same ? "" : `\n   live: ${lay.a}\n   moved: ${lay.b}`));

// drawn: every box shown and opened, every row opened -- the same rows and
// ref edges for both
const drawn = async (which) => {
  await ev(`(() => { last_event = ${which}; for (const d of layout(last_event).nodes) expanded.add(d.id); show_all_children(true); return 1; })()`);
  await ev(`settled()`);
  for (let i = 0; i < 6; i++) {
    const n = await ev(`(() => { const before = expanded.size;
      for (const t of document.querySelectorAll("text.row")) if (t.__data__ && t.__data__.key) expanded.add(t.__data__.key);
      redraw(); return expanded.size - before; })()`);
    await ev(`settled()`);
    if (!n) break;
  }
  return await ev(`(() => {
    // each box: its header sub-title (box_subtitle -- it once read view-model
    // keys, missed by ticket 17) and its rows
    const boxes = [...document.querySelectorAll("g.node")].map(g => { const sub = g.querySelector(":scope > text.sub");
      return [g.__data__.id, [sub && sub.getAttribute("display") !== "none" ? "sub: " + sub.textContent : "sub: -",
      ...[...g.querySelectorAll("text.row")].map(t => [...t.querySelectorAll(":scope > tspan")].map(x => x.firstChild ? x.firstChild.nodeValue : "").join(""))]]; });
    const edges = [...document.querySelectorAll("path.edge.member")].map(p => [p.__data__.target, [...p.__data__.row_keys].sort().join(",")]);
    return JSON.stringify({boxes: boxes.sort(), edges: edges.map(String).sort()}); })()`);
};
const a = await drawn("__orig");
const b = await drawn("__mo");
const A = JSON.parse(a), B = JSON.parse(b);
const n_rows = A.boxes.reduce((n, [, r]) => n + r.length, 0);
console.log(`   drawn: ${A.boxes.length} boxes, ${n_rows} rows, ${A.edges.length} ref edges`);
if (a !== b) {
  const bm = new Map(B.boxes);
  for (const [id, rows] of A.boxes)
    if (JSON.stringify(rows) !== JSON.stringify(bm.get(id)))
      console.log(`   box ${id}:\n     live:  ${JSON.stringify(rows).slice(0, 600)}\n     moved: ${JSON.stringify(bm.get(id)).slice(0, 600)}`);
  if (JSON.stringify(A.edges) !== JSON.stringify(B.edges))
    console.log(`   edges live-only: ${JSON.stringify(A.edges.filter(x => !B.edges.includes(x)))}\n   edges moved-only: ${JSON.stringify(B.edges.filter(x => !A.edges.includes(x)))}`);
}
check(a === b && A.boxes.length >= 10 && A.edges.length > 0, "drawn the same: boxes, rows and ref edges");

fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl1.close(); cl2.close(); process.exit(ok ? 0 : 1);
