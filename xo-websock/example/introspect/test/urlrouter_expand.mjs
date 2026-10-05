import fs from "node:fs";
const [,, cdp_port, port, out] = process.argv;
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
const cl = new WebSocket(`ws://localhost:${port}/`, "lws-minimal");
await new Promise(r => cl.onopen = r); cl.send('{"cmd":"subscribe","stream":"/demo/1"}');
await sleep(300);
const tgt = await (await fetch(`http://localhost:${cdp_port}/json/new?http://localhost:${port}/`, {method: "PUT"})).json();
const ws = new WebSocket(tgt.webSocketDebuggerUrl);
await new Promise(r => ws.onopen = r);
let seq = 0; const pending = new Map();
ws.onmessage = (ev) => { const m = JSON.parse(ev.data); if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); } };
const call = (method, params = {}) => new Promise(r => { const id = ++seq; pending.set(id, r); ws.send(JSON.stringify({id, method, params})); });
const ev = async (expr) => (await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true})).result.result?.value;
const VIS = `[...document.querySelectorAll("path.edge.member")].filter(p => [...document.querySelectorAll("text.row")].some(t => t.__data__ && p.__data__.row_keys.includes(t.__data__.key)))`;   // edges whose row is open
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok = " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1500, height: 1000, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`!!last_event && document.querySelectorAll("g.node").length >= 1`)); i++) await sleep(100);
await sleep(300);
// only the Webserver box is shown by default: show every box's children
await ev(`document.getElementById("show-all").click()`);
for (let i = 0; i < 100 && !(await ev(`document.querySelectorAll("path.edge").length > 0`)); i++) await sleep(100);
await sleep(500);
const box = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
const rows_of = async (id) => ev(`[...${box(id)}.querySelectorAll("text.row")].map(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join(""))`);
const click_row = async (id, prefix) => ev(`[...${box(id)}.querySelectorAll("text.row")].find(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join("").trimStart().startsWith(${JSON.stringify(prefix)})).dispatchEvent(new MouseEvent("click", {bubbles: true}))`);
const ends_from = async (from) => ev(`(() => { const boxes = [...document.querySelectorAll("g.node")].map(g => { const t = g.transform.baseVal[0].matrix, x = g.querySelector("rect"); return {id: g.__data__.id, x: t.e, y: t.f, w: +x.getAttribute("width"), h: +x.getAttribute("height")}; });
  return ${VIS}.filter(p => p.__data__.from === ${JSON.stringify(from)}).map(p => { const n = p.getAttribute("d").match(/-?[0-9.]+/g).map(Number); const ex = n[n.length-2], ey = n[n.length-1];
    const b = boxes.find(b => ex >= b.x - 2 && ex <= b.x + b.w + 2 && ey >= b.y - 2 && ey <= b.y + b.h + 2); return b ? b.id : null; }).sort(); })()`);

// (a) the url router is its own (nested) box: open it -> stream_map_ -> a
// row per stem, each an edge to its endpoint box
const U = "server/url_router_", R = "session:1/router_";
await ev(`${box(U)}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(700);
let r = await rows_of(U);
check(r.some(t => t.trimStart().startsWith("http_map_:") && t.endsWith("▸ {2}")) && r.some(t => t.trimStart().startsWith("stream_map_:") && t.endsWith("▸ {2}")),
      "the UrlRouter box opened: two maps, 2 stems each");
await click_row(U, "stream_map_:"); await sleep(700);
r = await rows_of(U);
console.log(" =  url router rows:", JSON.stringify(r));
check(r.some(t => t.trim() === '["/demo/"]: ▾ (→)') && r.some(t => t.trim() === '["/introspect"]: ▾ (→)'), "a row per stem, each a drawn ref");
let ends = await ends_from(U);
check(JSON.stringify(ends) === JSON.stringify(["stream:/demo/", "stream:/introspect"]), "map edges end at the stream endpoint boxes: " + JSON.stringify(ends));

// (b) session 1's router (its own box): its url_router_ refers to the
// server's url router -- the edge now ends on the UrlRouter box itself,
// not on the whole Webserver
await ev(`${box(R)}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(700);
r = await rows_of(R);
check(r.some(t => t.trimStart().startsWith("url_router_:") && t.endsWith("▾ (→)")), "the router's url_router_: a drawn ref, ▾ (→)");
ends = await ends_from(R);
// the router's one edge into the server is pjson_ -- the server's PrintJson,
// shared (printed in full in the server, a ref elsewhere: xo-printjson#02)
const pj_into_server = await ev(`${VIS}.filter(p => p.__data__.from === ${JSON.stringify(R)} && p.__data__.row_keys.some(k => /pjson_$/.test(k))).length`);
check(ends.includes(U) && ends.filter(e => e === "server").length === 1 && pj_into_server === 1,
      "its url_router_ edge ends on the UrlRouter box; the only edge into the server is pjson_: " + JSON.stringify(ends));
// ownership still decides the layering: the server above its sessions
const ys = await ev(`(() => { const y = id => [...document.querySelectorAll("g.node")].find(g => g.__data__.id === id).transform.baseVal[0].matrix.f; return {server: y("server"), s1: y("session:1"), s2: y("session:2")}; })()`);
check(ys.server < ys.s1 && ys.server < ys.s2, "server above its sessions: " + JSON.stringify(ys));
const overl = await ev(`(() => { const r = [...document.querySelectorAll("g.node > rect")].map(e => e.getBoundingClientRect()); let n = 0;
  for (let i = 0; i < r.length; i++) for (let j = i + 1; j < r.length; j++) { const a = r[i], b = r[j];
    if (a.left < b.right && b.left < a.right && a.top < b.bottom && b.top < a.bottom) n++; } return n; })()`);
check(overl === 0, "no overlaps");
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png", clip: {x: 0, y: 0, width: 1500, height: 1000, scale: 1}})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
