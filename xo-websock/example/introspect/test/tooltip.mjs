// tooltips: the page's own (#tip), coloured like the context menu -- not the
// browser's, which no stylesheet reaches.  No native tooltip remains
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
// headless chrome blurs the window ~2s after load; a blur hides the tooltip
await call("Emulation.setFocusEmulationEnabled", {enabled: true});
const mouse = (type, p, button = "none") => call("Input.dispatchMouseEvent", {type, x: p.x, y: p.y, button, clickCount: 1});
const center = (sel) => ev(`(() => { const r = ${sel}.getBoundingClientRect(); return {x: r.left + r.width / 2, y: r.top + r.height / 2}; })()`);
const G = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
const tip = () => ev(`(() => { const t = document.getElementById("tip"); return t && !t.hidden ? t.textContent : null; })()`);
const away = {x: 1400, y: 950};
await ev(`expanded.add("server"); redraw(); 1`); await ev(`settled()`);
check(await ev(`document.querySelectorAll("body title").length === 0 && document.querySelectorAll("body [title]").length === 0`),
      "no native tooltip: no <title>, no title=");
// a box: after a moment, its tooltip -- its type first
const box_tip = await ev(`${G("server")}.getAttribute("data-tip")`);
check(box_tip && box_tip.startsWith("xo::web::WebserverImpl"), "the server box's tooltip text: " + JSON.stringify(box_tip));
const lab = await center(`${G("server")}.querySelector(":scope > text.sub")`);
await mouse("mouseMoved", away); await sleep(100);
await mouse("mouseMoved", lab); await sleep(100);
check(await tip() === null, "not at once");
await sleep(600);
check(await tip() === box_tip, "then the box's tooltip: " + JSON.stringify(await tip()));
// coloured like the menu
const st = await ev(`(() => { const s = (id) => { const c = getComputedStyle(document.getElementById(id)); return [c.backgroundColor, c.color, c.borderTopColor, c.borderTopLeftRadius]; };
  return {tip: s("tip"), menu: s("ctxmenu")}; })()`);
check(JSON.stringify(st.tip) === JSON.stringify(st.menu), "background, text, border colour and corners as the menu's: " + JSON.stringify(st));
// near the pointer, below and right of it
const tr = await ev(`(() => { const r = document.getElementById("tip").getBoundingClientRect(); return {l: r.left, t: r.top, r: r.right, b: r.bottom}; })()`);
check(tr.l >= lab.x && tr.t > lab.y && tr.l - lab.x < 40 && tr.t - lab.y < 40, "below and right of the pointer: " + JSON.stringify({tr, lab}));
// a mouse press hides it
await mouse("mousePressed", lab, "left"); await mouse("mouseReleased", lab, "left"); await sleep(50);
check(await tip() === null, "a press hides it");
await ev(`settled()`);
await ev(`expanded.add("server"); redraw(); 1`); await ev(`settled()`);   // the press closed the box
// a ref arrow
const arrow = await center(`[...${G("server")}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === "ws_config").querySelector("tspan.rtoggle")`);
await mouse("mouseMoved", away); await sleep(100);
await mouse("mouseMoved", arrow); await sleep(700);
const at = await tip();
check(at && /^(Show|Hide) target ws_config :: /.test(at), "a ref arrow's tooltip: " + JSON.stringify(at));
// leaving hides it
await mouse("mouseMoved", away); await sleep(100);
check(await tip() === null, "leaving hides it");
// a toolbar button
const fit = await center(`document.getElementById("fit")`);
await mouse("mouseMoved", fit); await sleep(700);
check(await tip() === "the whole graph in view", "a toolbar button's tooltip: " + JSON.stringify(await tip()));
// a disabled menu item: its reason, over the menu
const mb = await center(`${G("server")}.querySelector(":scope > g.mbtn > rect")`);
await mouse("mouseMoved", mb); await mouse("mousePressed", mb, "left"); await mouse("mouseReleased", mb, "left"); await sleep(100);
const dis = await ev(`(() => { const b = [...document.querySelectorAll("#ctxmenu button")].find(b => b.disabled); return b ? b.dataset.tip : null; })()`);
const dc = await center(`[...document.querySelectorAll("#ctxmenu button")].find(b => b.disabled)`);
await mouse("mouseMoved", dc); await sleep(700);
check(dis && await tip() === dis, "a disabled menu item's reason: " + JSON.stringify([dis, await tip()]));
check(await ev(`+getComputedStyle(document.getElementById("tip")).zIndex > +getComputedStyle(document.getElementById("ctxmenu")).zIndex`), "drawn over the menu");
await mouse("mouseMoved", away); await sleep(100);
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); process.exit(ok ? 0 : 1);
