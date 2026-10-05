// redraws animate: boxes slide through intermediate positions; arrivals and
// edges fade in; leavers fade out; a quick second click interrupts cleanly
import fs from "node:fs";
const [,, cdp_port, port, out] = process.argv;
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
const cl = new WebSocket(`ws://localhost:${port}/`, "lws-minimal");
await new Promise(r => cl.onopen = r); cl.send('{"cmd":"subscribe","stream":"/demo/1"}'); await sleep(300);
const tgt = await (await fetch(`http://localhost:${cdp_port}/json/new?http://localhost:${port}/`, {method: "PUT"})).json();
const ws = new WebSocket(tgt.webSocketDebuggerUrl); await new Promise(r => ws.onopen = r);
let seq = 0; const pending = new Map();
ws.onmessage = (ev) => { const m = JSON.parse(ev.data); if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); } };
const call = (method, params = {}) => new Promise(r => { const id = ++seq; pending.set(id, r); ws.send(JSON.stringify({id, method, params})); });
const ev = async (expr) => { const r = await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true}); if (r.result.exceptionDetails) console.log("EXC", JSON.stringify(r.result.exceptionDetails).slice(0, 300)); return r.result.result?.value; };
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok  " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1500, height: 900, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`typeof last_event !== "undefined" && !!last_event && document.querySelectorAll("g.node").length >= 1`)); i++) await sleep(100);
await sleep(500);
const G = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
await ev(`document.getElementById("show-all").click(); 1`); await ev(`settled()`);
check(await ev(`[...document.querySelectorAll("g.node")].every(g => +getComputedStyle(g).opacity > 0.99)`), "after Show all settles, every box fully shown");

// sample a box's on-screen x every ~30ms in the page, through one redraw
const sample = async (id, action) => ev(`(async () => {
  const xs = []; const g = () => ${G(id)};
  const t0 = performance.now(); ${action};
  while (performance.now() - t0 < 1500) { const e = g(); if (e) xs.push(Math.round(e.getBoundingClientRect().top)); await new Promise(r => setTimeout(r, 30)); }
  return xs; })()`);
// opening the server (unanchored: called directly) moves the boxes below it
const xs = await sample("session:2", `expanded.add("server"); redraw()`);
const distinct = [...new Set(xs)];
console.log("   session:2 y over time:", JSON.stringify(distinct));
check(distinct.length >= 4, "session:2 passes through intermediate positions -- it slides, not jumps (" + distinct.length + " distinct)");
check(xs[0] === distinct[0] && xs[xs.length - 1] === distinct[distinct.length - 1] && Math.abs(distinct[0] - distinct[distinct.length - 1]) > 20, "from its old place to its new one");
await ev(`settled()`);

// edges fade in after the move: right after the draw they are transparent
const op = await ev(`(async () => { redraw(); await new Promise(r => setTimeout(r, 150)); return [...document.querySelectorAll("path.edge.member")].map(p => +getComputedStyle(p.parentNode).opacity); })()`);   // the fade is on its group (casing + edge)
check(op.length > 0 && op.every(o => o < 0.5), "edges faint while boxes move: " + JSON.stringify(op.slice(0, 4)));
await ev(`settled()`);
check(await ev(`[...document.querySelectorAll("path.edge.member")].every(p => +getComputedStyle(p).opacity > 0.99)`), "... and fully shown once settled");

// a leaving box fades out, then is gone
const fade = await ev(`(async () => { const key = [...wanted].find(k => k.endsWith('["/types"]')); hide_box("http:/types"); redraw();
  await new Promise(r => setTimeout(r, 60)); const g = ${G("http:/types")};
  const mid = g ? {leaving: g.classList.contains("leaving"), op: +getComputedStyle(g).opacity} : null;
  await settled(); return {mid, gone: !${G("http:/types")}}; })()`);
check(fade.mid && fade.mid.leaving && fade.mid.op < 1 && fade.gone, "a leaving box fades out, then is removed: " + JSON.stringify(fade));
// an arriving box fades in
const arrive = await ev(`(async () => { show_box("http:/types"); redraw();
  await new Promise(r => setTimeout(r, 60)); const g = ${G("http:/types")}; const early = g ? +getComputedStyle(g).opacity : null;
  await settled(); return {early, late: +getComputedStyle(${G("http:/types")}).opacity}; })()`);
check(arrive.early !== null && arrive.early < 0.5 && arrive.late > 0.99, "an arriving box fades in: " + JSON.stringify(arrive));

// a quick second click interrupts: opening then closing session 1 fast ends closed, placed, shown
await ev(`(async () => { expanded.add("session:1"); redraw(); await new Promise(r => setTimeout(r, 80)); expanded.delete("session:1"); redraw(); })()`);
await ev(`settled()`);
const fin = await ev(`(() => { const g = ${G("session:1")}; const m = g.transform.baseVal[0].matrix; const d = drawn_at.get("session:1");
  return {open: g.classList.contains("open"), rows: g.querySelectorAll("text.row").length, at: [m.e, m.f], want: [d.x, d.y], op: +getComputedStyle(g).opacity}; })()`);
check(!fin.open && fin.rows === 0 && fin.at[0] === fin.want[0] && fin.at[1] === fin.want[1] && fin.op > 0.99, "rapid open+close: ends closed, at its final place, shown: " + JSON.stringify(fin));
check(await ev(`document.querySelectorAll("g.node.leaving").length === 0 && [...document.querySelectorAll("g.node")].every(g => +getComputedStyle(g).opacity > 0.99)`), "no box left half-faded");
// opening / closing a row in place (a map, inside the UrlRouter's own box):
// the rows below slide with the outline -- no visible row ever outside its
// box (sampled every 20 ms)
const U = "server/url_router_";
const outside = async (action) => ev(`(async () => {
  const box = () => [...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(U)});
  let worst = 0, samples = 0; const t0 = performance.now(); ${action};
  while (performance.now() - t0 < 900) {
    const g = box(); const R = g.querySelector(":scope > rect").getBoundingClientRect();
    for (const t of g.querySelectorAll(":scope > g.rows > text.row")) {
      if (+getComputedStyle(t).opacity < 0.05) continue;    // not yet faded in
      const r = t.getBoundingClientRect();
      worst = Math.max(worst, r.bottom - R.bottom, r.right - R.right);
    }
    samples++; await new Promise(r => setTimeout(r, 20));
  }
  return {worst: Math.round(worst * 10) / 10, samples}; })()`);
await ev(`show_box(${JSON.stringify(U)}); expanded.add(${JSON.stringify(U)}); expanded.delete("${U}/http_map_"); expanded.delete("${U}/stream_map_"); redraw(); 1`); await ev(`settled()`);
let o = await outside(`expanded.add("${U}/http_map_"); redraw()`);
check(o.worst <= 1 && o.samples > 20, "opening http_map_ (in place): no row outside the box mid-move: " + JSON.stringify(o));
await ev(`settled()`);
o = await outside(`expanded.add("${U}/stream_map_"); redraw()`);
check(o.worst <= 1, "opening stream_map_ below it: none outside: " + JSON.stringify(o));
await ev(`settled()`);
o = await outside(`expanded.delete("${U}/http_map_"); redraw()`);
check(o.worst <= 1, "closing http_map_: the rows below slide up, none outside: " + JSON.stringify(o));
await ev(`settled()`);
const slid = await ev(`(async () => { const t = () => [...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(U)}).querySelectorAll("text.row");
  const row = () => [...t()].find(t => t.__data__.m._name_ === "stream_map_");
  expanded.add("${U}/http_map_"); redraw();
  const ys = []; const t0 = performance.now(); while (performance.now() - t0 < 700) { ys.push(Math.round(row().getBoundingClientRect().top)); await new Promise(r => setTimeout(r, 25)); }
  return [...new Set(ys)].length; })()`);
check(slid >= 4, "stream_map_'s row slides down through " + slid + " positions, not a jump");
await ev(`settled()`);
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
