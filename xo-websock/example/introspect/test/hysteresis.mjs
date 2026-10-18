// RC's repro (.xo-backlog/introspect-layout/spec.md): Show all, then open
// IntrospectReceiver.  Opening one leaf box must not move the others.
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
const R = "stream:/introspect:receiver";
const G = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
await ev(`document.getElementById("show-all").click(); 1`); await ev(`settled()`);
const before = new Map(await ev(`[...drawn_at]`));
check(before.size >= 10 && before.has(R), `Show all drew the graph, receiver included: ${before.size} boxes`);
// the user's click, on the box itself (not on screen coordinates: the legend may cover it)
await ev(`${G(R)}.dispatchEvent(new MouseEvent("click", {bubbles: true})); 1`); await ev(`settled()`);
check(await ev(`expanded.has(${JSON.stringify(R)})`), "the click opened the receiver");
const after = new Map(await ev(`[...drawn_at]`));
const ids = [...before.keys()].filter(k => k !== R && after.has(k));
const med = (xs) => { const s = [...xs].sort((a, b) => a - b); return s[Math.floor(s.length / 2)]; };
const sx = med(ids.map(k => after.get(k).x - before.get(k).x)), sy = med(ids.map(k => after.get(k).y - before.get(k).y));
const moved = ids.filter(k => Math.abs(after.get(k).x - before.get(k).x - sx) > 20 || Math.abs(after.get(k).y - before.get(k).y - sy) > 20);
const swaps = [];
for (const a of ids) for (const b of ids)
  if (a < b && Math.abs(before.get(a).y - before.get(b).y) < 5 && Math.abs(after.get(a).y - after.get(b).y) < 5
      && Math.sign(before.get(a).x - before.get(b).x) !== Math.sign(after.get(a).x - after.get(b).x))
    swaps.push([a, b]);
console.log(`   shared shift (${sx},${sy}); moved: ${JSON.stringify(moved)}; swaps: ${JSON.stringify(swaps)}`);
check(moved.length === 0, `no other box moves more than 20px beyond the shared shift: ${moved.length} of ${ids.length} did`);
check(swaps.length === 0, `no same-row pair swaps left/right: ${swaps.length} did`);
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); process.exit(ok ? 0 : 1);
