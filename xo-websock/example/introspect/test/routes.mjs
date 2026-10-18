// every member edge: orthogonal, from its source box's bottom edge to its
// target's top edge at its midpoint, through no other box
// (.xo-backlog/introspect-layout/spec.md)
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
await ev(`document.getElementById("show-all").click(); 1`); await ev(`settled()`);
const box = new Map(await ev(`[...drawn_at]`));
const P = 20;   // draw_aux's pad, local there (introspect.js:1380): drawn_at includes it, pts do not
const edges = await ev(`[...document.querySelectorAll("path.edge.member")].map(p => ({from: p.__data__.from, to: p.__data__.target, pts: p.__data__.pts.map(q => ({x: q.x + ${P}, y: q.y + ${P}}))}))`);
// problems with one route; [] if it keeps the contract
const problems = (e) => {
  const out = [], s = box.get(e.from), t = box.get(e.to), pts = e.pts, a = pts[0], z = pts[pts.length - 1];
  if (!s || !t) return [`no box for ${e.from} -> ${e.to}`];
  for (let i = 1; i < pts.length; i++)
    if (Math.abs(pts[i].x - pts[i - 1].x) > 0.5 && Math.abs(pts[i].y - pts[i - 1].y) > 0.5) out.push(`diagonal segment ${i}`);
  if (Math.abs(a.y - (s.y + s.h)) > 1.5 || a.x < s.x || a.x > s.x + s.w) out.push(`starts off its source's bottom edge: ${JSON.stringify(a)}`);
  if (Math.abs(z.y - t.y) > 1.5 || Math.abs(z.x - (t.x + t.w / 2)) > 1.5) out.push(`ends off its target's entry point: ${JSON.stringify(z)}`);
  for (const [id, b] of box) {
    if (id === e.from || id === e.to) continue;
    for (let i = 1; i < pts.length; i++) {   // a segment strictly inside the box, 1px in from its border
      const x0 = Math.min(pts[i - 1].x, pts[i].x), x1 = Math.max(pts[i - 1].x, pts[i].x);
      const y0 = Math.min(pts[i - 1].y, pts[i].y), y1 = Math.max(pts[i - 1].y, pts[i].y);
      if (x1 > b.x + 1 && x0 < b.x + b.w - 1 && y1 > b.y + 1 && y0 < b.y + b.h - 1) out.push(`crosses ${id}`);
    }
  }
  return out;
};
// the checker catches what it exists for: a diagonal through a box
const ids = [...box.keys()];
check(problems({from: ids[0], to: ids[1], pts: [{x: box.get(ids[0]).x, y: box.get(ids[0]).y}, {x: box.get(ids[1]).x + 7, y: box.get(ids[1]).y + 9}]}).length > 0, "self-check: a diagonal is reported");
check(edges.length >= 10, `Show all drew member edges: ${edges.length}`);
// [RF 2] at least one edge with an end inside a group (ELK reported those relative to the group) is in the run
check(await ev(`[...document.querySelectorAll("path.edge.member")].some(p => { const g = (id) => [...document.querySelectorAll("g.node")].find(n => n.__data__.id === id)?.__data__.group;
  return g(p.__data__.from) !== undefined || g(p.__data__.target) !== undefined; })`), "[RF 2] an edge with an end inside a group is covered");
const bad = edges.map(e => [e.from, e.to, problems(e)]).filter(x => x[2].length);
for (const b of bad) console.log("   ", JSON.stringify(b));
check(bad.length === 0, `every member edge keeps the route contract: ${bad.length} of ${edges.length} do not`);
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); process.exit(ok ? 0 : 1);
