// compact member rows: name = value; type on the name's tooltip and the row
// menu; ctrl/cmd-click the name opens source; "types" brings types inline.
// Run against a server started with --src-tree (source links on).
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
const ev = async (expr) => { const r = await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true}); if (r.result.exceptionDetails) console.log("EXC", JSON.stringify(r.result.exceptionDetails).slice(0, 300)); return r.result.result?.value; };
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok  " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1500, height: 1000, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`!!last_event && document.querySelectorAll("g.node").length >= 1 && src.link !== null`)); i++) await sleep(100);
await sleep(600);
await ev(`window.__opened = []; window.open = (u) => { window.__opened.push(u); return null; }; 1`);
await ev(`document.getElementById("show-all").click()`); await sleep(900);
const G = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
const ROW = (id, name) => `[...${G(id)}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === ${JSON.stringify(name)})`;
const rowtext = `t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join("")`;
const rows = async (id) => ev(`[...${G(id)}.querySelectorAll("text.row")].map(${rowtext})`);
const U = "server/url_router_";   // the server's UrlRouter: its own (nested) box
const width = async (id) => ev(`+${G(id)}.querySelector("rect").getAttribute("width")`);

await ev(`${G("server")}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(800);
// pan (zoom kept at 100%: the checks measure pixels) so the UrlRouter box
// sits well inside the viewport -- the real mouse moves below reach it
await ev(`(() => { const b = drawn_at.get("${U}"); d3.select("#graph").call(zoom.transform, d3.zoomIdentity.translate(500 - b.x, 250 - b.y)); })()`); await sleep(200);
await ev(`${G(U)}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(800);
await ev(`${ROW(U, "stream_map_")}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(800);

// 1. compact rows
let r = await rows("server");
console.log("   rows:", JSON.stringify(r.slice(0, 6)));
check(r.includes("listen_port_: " + port) && r.some(t => t.trim() === "url_router_: ▾ (→)"), "rows read name: value; url_router_ a ref to its own box");
check(!r.some(t => t.includes("[atomic]") || t.includes("[struct]") || t.includes("atomic<int>")), "no type or metatype inline");
check(await ev(`${G("server")}.querySelectorAll("tspan.mtype, tspan.mtag").length`) === 0, "no type tspans");

// 1b. the separator at x0 + indent * depth: steps in with nesting, as the names do
await ev(`${ROW(U, "http_map_")}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(800);
const eqs = await ev(`[...${G(U)}.querySelectorAll("text.row")].map(t => {
    const eq = t.querySelector(":scope > tspan.meq").getBoundingClientRect(), nm = t.querySelector(":scope > tspan.mname").getBoundingClientRect();
    return {depth: t.__data__.depth, x: eq.left, gap: eq.left - nm.right}; })`);
const x0s = eqs.map(e => e.x - 14 * e.depth);
console.log("   separator x by depth:", JSON.stringify(eqs.map(e => [e.depth, Math.round(e.x)])));
check(Math.max(...x0s) - Math.min(...x0s) < 0.5, "every row's separator at x0 + 14 * depth (x0 spread " + (Math.max(...x0s) - Math.min(...x0s)).toFixed(2) + ")");
// nested structs have boxes of their own now, so live rows go 2 deep at most
check(new Set(eqs.map(e => e.depth)).size >= 2, "rows at 2+ depths, so the step is exercised");
const gaps = eqs.map(e => e.gap);
check(gaps.every(g => Math.abs(g) < 1), "names right-justified: every name ends at its separator (gaps " + Math.min(...gaps).toFixed(2) + ".." + Math.max(...gaps).toFixed(2) + ")");
const starts = await ev(`[...${G(U)}.querySelectorAll("text.row")].map(t => ({d: t.__data__.depth, x: t.querySelector(":scope > tspan.mname").getBoundingClientRect().left - ${G(U)}.querySelector(":scope > rect").getBoundingClientRect().left}))`);
check(starts.every(s => s.x >= 12 + 14 * s.d - 1), "no name starts left of its depth's indent");
check(starts.some(s => Math.abs(s.x - (12 + 14 * s.d)) < 1), "the widest name sits at its indent (x0 the least that fits)");
await ev(`${ROW(U, "http_map_")}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(800);

// 1c. a ref row: ▾ (→); what it refers to is the value's tooltip
r = await rows(U);
check(r.some(t => t.trim() === '["/introspect"]: ▾ (→)'), "ref row reads ▾ (→)");
const rtip = await ev(`[...${ROW(U, '["/introspect"]')}.querySelectorAll(":scope > tspan.mval")].map(ts => ts.querySelector("title")).filter(x => x).map(x => x.textContent)[0]`);
check(/^refers to \/introspect\n[0-9]+$/.test(rtip), "its value's tooltip names the target and its id: " + JSON.stringify(rtip));

// 1d. row triangles: 25% larger, each with a square behind -- plain until
// hovered; clicking the square does what clicking the triangle does
const tri = await ev(`(() => { const g = ${G(U)}.querySelector(":scope > g.rows");
  const tris = [...g.querySelectorAll("text.row > tspan.tri")], rects = [...g.querySelectorAll(":scope > rect.tbtn")];
  return {n_tri: tris.length, n_rect: rects.length, size: getComputedStyle(tris[0]).fontSize, row_size: getComputedStyle(tris[0].parentNode).fontSize,
          op: getComputedStyle(rects[0]).fillOpacity, w: +rects[0].getAttribute("width"), h: +rects[0].getAttribute("height")}; })()`);
console.log("   triangles:", JSON.stringify(tri));
check(tri.n_tri > 0 && tri.n_tri === tri.n_rect, "a square per triangle");
check(parseFloat(tri.size) === parseFloat(tri.row_size) * 1.25, "triangles at 125% of the row's font");
check(tri.op === "0" && tri.w === tri.h, "squares are square, and plain until hovered");
const sq = await ev(`(() => { const ts = ${ROW(U, "stream_map_")}.querySelector("tspan.tri"); const r = ts.getBoundingClientRect(); return {x: r.left + r.width / 2, y: r.top + r.height / 2}; })()`);
await call("Input.dispatchMouseEvent", {type: "mouseMoved", x: sq.x, y: sq.y, button: "none"}); await sleep(150);
check(await ev(`getComputedStyle(${G(U)}.querySelector("rect.tbtn.hot")).fillOpacity`) === "0.85", "hovering the triangle shows its square (white, 85%)");
await call("Input.dispatchMouseEvent", {type: "mouseMoved", x: 5, y: 5, button: "none"}); await sleep(150);
check(await ev(`${G(U)}.querySelectorAll("rect.tbtn.hot").length`) === 0, "... and leaving hides it");
// click the square (not the glyph): toggles that row
const rect_of = (name) => `(() => { const ts = ${ROW(U, name)}.querySelector("tspan.tri"); const b = ts.getBBox();
  return [...${G(U)}.querySelectorAll("rect.tbtn")].find(r => Math.abs(+r.getAttribute("x") + +r.getAttribute("width") / 2 - (b.x + b.width / 2)) < 0.5
                                                                 && Math.abs(+r.getAttribute("y") + +r.getAttribute("height") / 2 - (b.y + b.height / 2)) < 0.5); })()`;
await ev(`${rect_of("stream_map_")}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(800);
check(!(await ev(`expanded.has("server/url_router_/stream_map_")`)), "clicking stream_map_'s square closes it");
await ev(`${rect_of("stream_map_")}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(800);
check(await ev(`expanded.has("server/url_router_/stream_map_")`), "... and again opens it");
await ev(`${rect_of('["/introspect"]')}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(800);
check(!(await ev(`wanted.has('server/url_router_/stream_map_/["/introspect"]')`)), "a ref row's square unwants its edge");
await ev(`${rect_of('["/introspect"]')}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(800);
check(await ev(`wanted.has('server/url_router_/stream_map_/["/introspect"]')`), "... and again wants it");
check(await ev(`getComputedStyle(document.querySelector("g.node.server > text.kids")).fontSize`) === "16px", "the box's children triangle is larger too (16px)");
check(await ev(`document.querySelectorAll("g.node > rect.tbtn, svg > rect.tbtn").length`) === 0, "... with no square: only triangles inside boxes get one");

// 1e. ws_config_: a struct with members -- its own box, the row a ref to it
check((await rows("server")).some(t => t.trim() === "ws_config_: ▾ (→)"), "ws_config_ row: a ref to its own box");
await ev(`${G("server/ws_config_")}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await ev(`settled()`);
const cfg_rows = await rows("server/ws_config_");
console.log("   ws_config_:", JSON.stringify(cfg_rows));
check(cfg_rows.some(t => t.trim() === "port_: " + port) && cfg_rows.some(t => t.trim() === "tls_flag_: false") && cfg_rows.some(t => t.includes("mount_origin_: ")),
      "its box's rows: port_, tls_flag_, ..., mount_origin_");
await ev(`${G("server/ws_config_")}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await ev(`settled()`);

// 2. tooltip on the name
const tip = await ev(`${ROW("server", "url_router_")}.querySelector("tspan.mname > title").textContent`);
console.log("   tooltip:", JSON.stringify(tip));
check(tip.startsWith("url_router_: UrlRouter  [struct]\nxo::web::UrlRouter\n") && /UrlRouter\.hpp:\d+/.test(tip) && tip.endsWith("ctrl-click: open source"),
      "name tooltip: short type, metatype, canonical type, source, ctrl-click hint");

// 3. ctrl-click the name: opens source, does not toggle the row
await ev(`${ROW("server", "url_router_")}.querySelector("tspan.mname").dispatchEvent(new MouseEvent("click", {bubbles: true, ctrlKey: true}))`); await sleep(300);
let opened = await ev(`window.__opened`);
check(opened.length === 1 && /UrlRouter\.hpp#L\d+$/.test(opened[0]), "ctrl-click opens its source: " + JSON.stringify(opened));
check(await ev(`wanted.has("server/url_router_")`), "... and leaves its edge wanted");
await ev(`${ROW("server", "url_router_")}.querySelector("tspan.mname").dispatchEvent(new MouseEvent("click", {bubbles: true, metaKey: true}))`); await sleep(300);
check((await ev(`window.__opened`)).length === 2, "cmd-click too");

// 4. row menu: right-click a row -- not the box's menu
const menu = async () => ev(`({hidden: document.getElementById("ctxmenu").hidden, head: document.querySelector("#ctxmenu .ctxhead").textContent,
  items: [...document.querySelectorAll("#ctxmenu button")].map(b => b.textContent + (b.disabled ? " (disabled: " + b.title + ")" : ""))})`);
await ev(`${ROW("server", "url_router_")}.dispatchEvent(new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: 300, clientY: 300}))`); await sleep(200);
let m = await menu();
console.log("   url_router_ menu:", JSON.stringify(m));
check(!m.hidden && m.head === "url_router_: UrlRouter", "row menu, headed name: Type");
check(JSON.stringify(m.items) === JSON.stringify(["Open source", "Copy type name", "Hide ▸ UrlRouter"]), "a ref to its own box: Open source, Copy type name, Hide ▸ UrlRouter: " + JSON.stringify(m.items));
await ev(`[...document.querySelectorAll("#ctxmenu button")].find(b => b.textContent === "Open source").click()`); await sleep(200);
check((await ev(`window.__opened`)).length === 3, "menu Open source opens it");

// a member opened in place: Collapse
await ev(`document.body.dispatchEvent(new KeyboardEvent("keydown", {key: "Escape", bubbles: true}))`); await sleep(100);
await ev(`${ROW(U, "stream_map_")}.dispatchEvent(new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: 300, clientY: 300}))`); await sleep(200);
m = await menu();
check(m.head.startsWith("stream_map_: ") && m.items[m.items.length - 1] === "Collapse", "stream_map_ (open in place): its menu ends Collapse: " + JSON.stringify(m.items));

// a ref row: no declared type; Hide ▸ / Show ▸ its target
await ev(`${ROW(U, '["/introspect"]')}.dispatchEvent(new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: 300, clientY: 300}))`); await sleep(200);
m = await menu();
console.log("   [\"/introspect\"] menu:", JSON.stringify(m));
check(m.items[0] === "Open source (disabled: no declared type)" && m.items.includes("Hide ▸ /introspect"), "element row: Open source disabled; Hide ▸ its target");
await ev(`[...document.querySelectorAll("#ctxmenu button")].find(b => b.textContent === "Hide ▸ /introspect").click()`); await sleep(800);
check(!(await ev(`wanted.has('server/url_router_/stream_map_/["/introspect"]')`)), "Hide ▸ unwants its edge");
await ev(`${ROW(U, '["/introspect"]')}.dispatchEvent(new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: 300, clientY: 300}))`); await sleep(200);
check((await menu()).items.includes("Show ▸ /introspect"), "... and the entry flips to Show ▸");
await ev(`document.body.dispatchEvent(new KeyboardEvent("keydown", {key: "Escape", bubbles: true}))`); await sleep(100);

// the box header still gives the box menu
await ev(`${G("server")}.dispatchEvent(new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: 300, clientY: 300}))`); await sleep(200);
check((await menu()).head === "xo::web::WebserverImpl", "right-click elsewhere on the box: the box menu");
await ev(`document.body.dispatchEvent(new KeyboardEvent("keydown", {key: "Escape", bubbles: true}))`); await sleep(100);

// 5. types, from the box menu: inline again, wider
const types_menu = async (label) => {
    await ev(`${G("server")}.dispatchEvent(new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: 300, clientY: 300}))`); await sleep(200);
    check((await menu()).items.includes(label), `box menu offers ${label}`);
    await ev(`[...document.querySelectorAll("#ctxmenu button")].find(b => b.textContent === ${JSON.stringify(label)}).click()`); await sleep(900);
};
const w_compact = await width("server");
await types_menu("Show types");
r = await rows("server");
check(r.some(t => t === "listen_port_: atomic<int> [atomic]") && r.some(t => t.startsWith("url_router_: ") && t.endsWith(" UrlRouter [struct] (→)")),
      "types on: name: Type [metatype], the value dropped (a ref keeps its arrow): " + JSON.stringify(r.slice(0, 3)));
const value_tip = await ev(`[...${G("server")}.querySelectorAll("text.row > tspan.mname")].find(ts => ts.firstChild.nodeValue === "listen_port_").querySelector("title").textContent`);
check(/^= \d+\n/.test(value_tip), "the value moves to the name's tooltip: " + JSON.stringify(value_tip.split("\n")[0]));
const w_types = await width("server");
check(w_types > w_compact * 1.5, `Webserver box ${Math.round(w_compact)}px compact, ${Math.round(w_types)}px with types`);
await types_menu("Hide types");
check(await width("server") === w_compact, "types off again: back to compact");

fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png", clip: {x: 0, y: 0, width: 1500, height: 1000, scale: 1}})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
