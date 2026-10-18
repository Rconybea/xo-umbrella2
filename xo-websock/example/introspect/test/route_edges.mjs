// route_edges (.xo-backlog/introspect-layout/spec.md): libavoid behind one
// function, on synthetic boxes
import fs from "node:fs";
const [,, cdp_port, port, out] = process.argv;
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
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
const r = await ev(`(async () => {
  const A = await load_router();
  const orth = (pts) => pts.every((p, i) => i === 0 || Math.abs(p.x - pts[i - 1].x) < 0.5 || Math.abs(p.y - pts[i - 1].y) < 0.5);
  const crosses = (pts, b) => pts.some((p, i) => i > 0 && Math.max(p.x, pts[i-1].x) > b.x + 1 && Math.min(p.x, pts[i-1].x) < b.x + b.w - 1
                                                     && Math.max(p.y, pts[i-1].y) > b.y + 1 && Math.min(p.y, pts[i-1].y) < b.y + b.h - 1);
  const out = {};
  // (a) a wide box between source and target: routed around it
  const boxes = new Map([["s", {x: 0, y: 0, w: 100, h: 40}], ["m", {x: -20, y: 100, w: 240, h: 40}], ["t", {x: 0, y: 200, w: 100, h: 40}]]);
  const a = route_edges(A, boxes, [{key: "k", from: "s", out_x: 50, to: "t"}], 24).get("k");
  out.a = {pts: a, orth: orth(a), start: a[0], end: a[a.length - 1], crosses: crosses(a, boxes.get("m"))};
  // (b) [RF 3] two edges into one box: both orthogonal, both at its one entry point
  const b2 = new Map([["s1", {x: 0, y: 0, w: 100, h: 40}], ["s2", {x: 300, y: 0, w: 100, h: 40}], ["t", {x: 150, y: 200, w: 100, h: 40}]]);
  const rb = route_edges(A, b2, [{key: "k1", from: "s1", out_x: 50, to: "t"}, {key: "k2", from: "s2", out_x: 50, to: "t"}], 24);
  out.b = [...rb.values()].map(p => ({orth: orth(p), end: p[p.length - 1]}));
  // (c) [RF 4] an edge from a box to itself: no throw, orthogonal
  try { const c = route_edges(A, new Map([["s", {x: 0, y: 0, w: 100, h: 40}]]), [{key: "k", from: "s", out_x: 50, to: "s"}], 24).get("k");
        out.c = {orth: orth(c), n: c.length}; } catch (e) { out.c = {threw: String(e)}; }
  // (d) nothing to route
  out.d = route_edges(A, boxes, [], 24).size;
  // (e) still alive after (c): the wasm did not abort
  out.e = route_edges(A, boxes, [{key: "k", from: "s", out_x: 50, to: "t"}], 24).get("k").length;
  return out; })()`);
console.log("   ", JSON.stringify(r));
check(r.a.orth && !r.a.crosses, "(a) orthogonal, around the box in between");
check(r.a.start.x === 50 && r.a.start.y === 40, "(a) leaves the source's bottom edge at its out-port: " + JSON.stringify(r.a.start));
check(r.a.end.x === 24 && r.a.end.y === 200, "(a) enters the target's top edge at in_x: " + JSON.stringify(r.a.end));
check(r.b.length === 2 && r.b.every(x => x.orth && x.end.x === 174 && x.end.y === 200), "[RF 3] (b) two edges into one box: orthogonal, one entry point");
check(r.c && !r.c.threw && r.c.orth, "[RF 4] (c) an edge from a box to itself: " + JSON.stringify(r.c));
check(r.d === 0, "(d) no edges: an empty map");
check(r.e >= 2, "(e) the router still works after (c)");
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); process.exit(ok ? 0 : 1);
