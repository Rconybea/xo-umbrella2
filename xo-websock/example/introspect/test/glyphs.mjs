// a ref row's glyph says where its target is: → not drawn (click: show it),
// ↴ drawn (click: hide it); the Webserver, always drawn, an inert ↴; an
// object no box draws, a dimmed, inert →.  Rows that open in place keep
// their triangles
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
const G = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
// the glyph after a row's "name: ": its text, classes, tooltip, colour
const glyph = (box, name) => ev(`(() => { const t = [...${G(box)}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === ${JSON.stringify(name)});
  if (!t) return null; const g = t.querySelectorAll(":scope > tspan")[2];
  return {text: g.textContent, cls: g.getAttribute("class"),
          tip: g.getAttribute("data-tip") || "", fill: getComputedStyle(g).fill,
          row: [...t.querySelectorAll(":scope > tspan")].map(x => x.firstChild ? x.firstChild.nodeValue : "").join("")}; })()`);
// 1. a box that can be shown, not drawn: →, a toggle
await ev(`expanded.add("server"); redraw(); 1`); await ev(`settled()`);
let g = await glyph("server", "ws_config");
check(g && g.text === "→" && /\btri\b/.test(g.cls) && /\brtoggle\b/.test(g.cls), "not drawn: →, a toggle: " + JSON.stringify(g));
check(g && g.row === "ws_config: →", "no (→) after it: " + JSON.stringify(g && g.row));
const ws_type = await ev(`[...${G("server")}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === "ws_config").__data__.m._short_type_`);
check(g && g.tip.startsWith(`Show target ws_config :: ${ws_type}\n`), "its tooltip: Show target <member> :: <type>, then more: " + JSON.stringify(g && g.tip));
// 2. drawn: ↴, still a toggle
await ev(`document.getElementById("show-all").click(); 1`); await ev(`settled()`);
await ev(`expanded.add("server"); redraw(); 1`); await ev(`settled()`);
g = await glyph("server", "ws_config");
check(g && g.text === "↴" && /\brtoggle\b/.test(g.cls), "drawn: ↴, a toggle: " + JSON.stringify(g));
check(g && g.tip.startsWith(`Hide target ws_config :: ${ws_type}\n`), "its tooltip: Hide target <member> :: <type>, then more: " + JSON.stringify(g && g.tip));
// 3. the Webserver, always drawn: ↴, inert -- no toggle, no button
const R = "stream:/introspect:receiver";
await ev(`expanded.add(${JSON.stringify(R)}); redraw(); 1`); await ev(`settled()`);
g = await glyph(R, "websrv");
check(g && g.text === "↴" && !/\btri\b/.test(g.cls), "the server: ↴, inert: " + JSON.stringify(g));
const normal = g && g.fill;
// 4. an object no box draws: a dimmed, inert →
await ev(`(() => { const e = JSON.parse(JSON.stringify(last_event));
  member_value(e, "server")._members_.push({_name_: "ghost_", _canonical_type_: "xo::web::Ghost *", _short_type_: "Ghost *", _metatype_: "pointer", _value_: {_ref_: 987654321}});
  last_event = e; draw(e); return 1; })()`); await ev(`settled()`);
g = await glyph("server", "ghost_");
check(g && g.text === "→" && !/\btri\b/.test(g.cls) && /\bdim\b/.test(g.cls) && g.fill !== normal, "no box draws it: a dimmed, inert →: " + JSON.stringify(g) + " vs " + normal);
check(g && /not drawn/.test(g.tip), "its tooltip says not drawn: " + JSON.stringify(g && g.tip));
// rows that open in place keep their triangles
check(await ev(`[...document.querySelectorAll("text.row tspan.xtoggle")].every(t => t.textContent.startsWith("▸") || t.textContent.startsWith("▾"))`), "in-place rows keep ▸ / ▾");
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); process.exit(ok ? 0 : 1);
