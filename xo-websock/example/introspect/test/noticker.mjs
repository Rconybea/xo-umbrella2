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
  return [...document.querySelectorAll("path.edge.member")].filter(p => p.__data__.from === ${JSON.stringify(from)}).map(p => { const n = p.getAttribute("d").match(/-?[0-9.]+/g).map(Number); const ex = n[n.length-2], ey = n[n.length-1];
    const b = boxes.find(b => ex >= b.x - 2 && ex <= b.x + b.w + 2 && ey >= b.y - 2 && ey <= b.y + b.h + 2); return b ? b.id : null; }).sort(); })()`);

// no ticker box, no holds edges
check(await ev(`document.querySelectorAll("g.node.app").length`) === 0, "no ticker box");
check(await ev(`document.querySelectorAll("path.edge.holds").length`) === 0, "no holds edges");
// refcount badges dropped (RC: insufficiently interesting); app_holds, which
// only fed their accounting, gone from the snapshot
check(await ev(`document.querySelectorAll("g.badge").length`) === 0, "no refcount badges");
check(await ev(`!("app_holds" in last_event)`), "no app_holds in the snapshot");
check(await ev(`typeof member_value(last_event, "server").refcount === "number"`), "the printers' refcount fields remain (Show JSON)");
// the legend, on the graph's top-left corner: one entry per colour, top
// down, the snapshot's short type; swatch = box.  Nested boxes: a colour
// per type, in the order the types first appear
const leg = await ev(`[...document.querySelectorAll("g.legend g.entry")].map(e => { const r = e.querySelector("rect").getBoundingClientRect();
  return {type: e.querySelector("text").textContent, x: Math.round(r.left), y: Math.round(r.top),
          fill: getComputedStyle(e.querySelector("rect")).fill, stroke: getComputedStyle(e.querySelector("rect")).stroke}; })`);
console.log("   legend:", JSON.stringify(leg.map(e => e.type)));
const nested_types = ["WebserverConfig", "UrlRouter", "WsSessionTable<WebsocketSessionRecd>", "WsSessionRouter"];
check(JSON.stringify(leg.map(e => e.type)) === JSON.stringify(["WebserverImpl", "DynamicEndpoint", "IntrospectReceiver", "WebsocketSessionRecd", "WsSessionSender<WebserverImpl>", "Subscription", "WebsocketSinkImpl", ...nested_types]),
      "legend: the seven types, then each nested struct's type");
check(leg.every((e, i) => e.x === leg[0].x && (i === 0 || e.y > leg[i - 1].y)), "entries stacked vertically, one column");
const box_fill = async (kind) => ev(`(() => { const g = document.querySelector("g.node.${kind} > rect"); return g ? getComputedStyle(g).fill : null; })()`);
check(leg[0].fill === await box_fill("server") && leg[1].fill === await box_fill("http") && leg[5].fill === await box_fill("subscription"),
      "each swatch has its boxes' fill");
check(await box_fill("http") === await box_fill("stream"), "http and stream endpoints: one colour (one C++ type)");
const nested_leg = leg.slice(7);
check(new Set(nested_leg.map(e => e.fill)).size === 4 && new Set(nested_leg.map(e => e.stroke)).size === 4
      && !nested_leg.some(e => leg.slice(0, 7).some(k => k.fill === e.fill)),
      "the nested types: four distinct colours, none a parent kind's: " + JSON.stringify(nested_leg.map(e => e.fill)));
await ev(`document.getElementById("show-all").click(); 1`); await ev(`settled()`);
const nested_boxes = await ev(`[...document.querySelectorAll("g.node.nested")].map(g => [g.__data__.label, getComputedStyle(g.querySelector(":scope > rect")).fill, getComputedStyle(g.querySelector(":scope > rect")).stroke])`);
check(nested_boxes.length === 5 && nested_boxes.every(([t, f, s]) => nested_leg.some(e => e.type === t && e.fill === f && e.stroke === s)),
      "each nested box has its type's swatch colours (both routers alike): " + JSON.stringify(nested_boxes));
// fixed on the viewport's top-left corner: the camera moves, the legend doesn't
const leg_at = `(() => { const v = document.getElementById("graph").getBoundingClientRect(), r = document.querySelector("g.legend > rect.panel").getBoundingClientRect();
  return {dx: Math.round(r.left - v.left), dy: Math.round(r.top - v.top), w: Math.round(r.width)}; })()`;
const la = await ev(leg_at);
check(la.dx === 9 && la.dy === 9, "legend panel 8px in from the viewport's top-left (+1: its border): " + JSON.stringify(la));
check(await ev(`document.querySelector("#graph > g.legend") === document.querySelector("#graph").lastElementChild && !document.querySelector("g.camera g.legend")`), "drawn over the drawing, outside the camera");
await ev(`d3.select("#graph").call(zoom.transform, d3.zoomIdentity.translate(300, 120).scale(1.7)); 1`); await sleep(200);
check(JSON.stringify(await ev(leg_at)) === JSON.stringify(la), "pan + zoom: the legend stays put, its size too");
await ev(`document.getElementById("fit").click()`); await sleep(700);
check(await ev(`(() => { const l = document.querySelector("g.legend > rect.panel").getBoundingClientRect(), c = document.querySelector("#graph > g.extent > rect, #graph > g.camera > g.extent > rect").getBoundingClientRect();
  return c.left >= l.right - 0.5 || c.top >= l.bottom - 0.5; })()`), "Fit: the drawing clear of the legend");
// fit_camera's cases, on made-up drawing sizes in this viewport
const fc = await ev(`(() => { const {width: W, height: H} = view_size(); const f = (w, h) => { const t = fit_camera({w, h}); return {x: t.x, y: t.y, k: t.k, w, h}; };
  return {W, H, lw: legend_w, lh: legend_h,
          wide: f(4 * W, H / 2),           // width-bound, centred: well below the legend
          tall: f(W / 4, 4 * H),           // height-bound, centred: well right of it
          slide: f(0.7 * W, H),            // centred would overlap; slack enough to slide right
          shrink: f(W, H),                 // no slack: smaller, right of or below the legend
          tiny: f(20, 10)}; })()`);
console.log("   fit cases:", JSON.stringify(fc));
const near = (a, b) => Math.abs(a - b) < 0.5;
check(near(fc.wide.k, fc.W / fc.wide.w) && near(fc.wide.x, 0) && near(fc.wide.y, (fc.H - fc.wide.k * fc.wide.h) / 2), "wide: fills the width, centred (as Center)");
check(near(fc.tall.k, fc.H / fc.tall.h) && near(fc.tall.y, 0) && near(fc.tall.x, (fc.W - fc.tall.k * fc.tall.w) / 2), "tall: fills the height, centred");
check(near(fc.slide.k, 1) && near(fc.slide.x, fc.lw) && near(fc.slide.y, 0), "would overlap the legend, room to slide: same size, moved right of it");
const right_k = Math.min((fc.W - fc.lw) / fc.W, 1), below_k = Math.min(1, (fc.H - fc.lh) / fc.H);
check(near(fc.shrink.k, Math.max(right_k, below_k)) && (fc.shrink.x >= fc.lw - 0.5 || fc.shrink.y >= fc.lh - 0.5), "no room: as large as fits right of / below the legend: " + JSON.stringify(fc.shrink));
check(fc.tiny.k === 3, "a tiny drawing: the zoom's maximum, 3");
check(!(await ev(`document.getElementById("legend")`)), "no legend above the graph any more");
// the drawing's extent: a white sheet, the size Fit / Center use, first in
// the camera (under groups, edges, boxes), pan / zoom with the drawing
const ex = await ev(`(() => { const r = document.querySelector("#graph > g.camera > g.extent > rect"); const cam = document.querySelector("#graph > g.camera");
  return r && {w: +r.getAttribute("width"), h: +r.getAttribute("height"), ds: drawing_size, first: cam.firstElementChild === r.parentNode,
               fill: getComputedStyle(r).fill, bg: getComputedStyle(document.getElementById("graph")).backgroundColor, casing: getComputedStyle(document.querySelector("path.casing")).stroke}; })()`);
console.log("   extent:", JSON.stringify(ex));
check(ex && Math.abs(ex.w - ex.ds.w) < 0.5 && Math.abs(ex.h - ex.ds.h) < 0.5 && ex.first, "extent sheet: drawing_size, under everything in the camera");
check(ex.fill === "rgb(255, 255, 255)" && ex.bg !== ex.fill && ex.casing === ex.fill, "white on the viewport's grey; edge casing the sheet's colour");
const sheet_vs_cam = await ev(`(() => { const a = document.querySelector("g.extent > rect").getBoundingClientRect(), t = d3.zoomTransform(document.getElementById("graph")), v = document.getElementById("graph").getBoundingClientRect();
  return Math.abs(a.left - (v.left + 1 + t.x)) < 1.5 && Math.abs(a.width - t.k * drawing_size.w) < 1.5; })()`);
check(sheet_vs_cam, "the sheet moves and scales with the camera");
// the "legend" checkbox: on by default; off hides it, and Fit then uses
// the whole viewport
check(await ev(`document.getElementById("show-legend").checked`) && await ev(`document.querySelector("g.legend").getAttribute("display")`) === null, "legend checkbox on by default, legend drawn");
await ev(`document.getElementById("show-legend").click(); 1`); await ev(`settled()`);
check(await ev(`document.querySelector("g.legend").getAttribute("display")`) === "none" && await ev(`legend_w`) === 0, "unchecked: legend hidden, no column reserved");
await ev(`document.getElementById("fit").click()`); await sleep(700);
check(await ev(`(() => { const t = d3.zoomTransform(document.getElementById("graph")), c = centred(drawing_size, t.k); return Math.abs(t.x - c.x) < 0.5 && Math.abs(t.y - c.y) < 0.5; })()`), "... Fit: centred, as Center would place it");
await ev(`document.getElementById("show-legend").click(); 1`); await ev(`settled()`);
check(await ev(`document.querySelector("g.legend").getAttribute("display")`) === null && await ev(`legend_w`) > 100, "checked again: legend back");
check(await ev(`(() => { const v = document.getElementById("graph").getBoundingClientRect(); return v.bottom <= innerHeight + 1; })()`), "the viewport still ends within the window");
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
