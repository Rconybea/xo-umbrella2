// the box you click in stays put on screen; the layout moves around it
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
const mouse = async (type, x, y, button, modifiers = 0) => call("Input.dispatchMouseEvent", {type, x, y, button, clickCount: 1, modifiers});
const SHIFT = 8;   // CDP modifier bit
const click_at = async (p) => { await mouse("mouseMoved", p.x, p.y, "none"); await mouse("mousePressed", p.x, p.y, "left"); await mouse("mouseReleased", p.x, p.y, "left"); };
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok  " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1500, height: 900, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`typeof last_event !== "undefined" && !!last_event && document.querySelectorAll("g.node").length >= 1`)); i++) await sleep(100);
await sleep(500);
const G = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
const at = async (id) => ev(`(() => { const g = ${G(id)}; if (!g) return null; const r = g.querySelector(":scope > rect").getBoundingClientRect(); return {x: Math.round(r.left * 10) / 10, y: Math.round(r.top * 10) / 10}; })()`);
const center = async (sel) => ev(`(() => { const r = ${sel}.getBoundingClientRect(); return {x: r.left + r.width / 2, y: r.top + r.height / 2}; })()`);
const same = (a, b) => a && b && Math.abs(a.x - b.x) < 1 && Math.abs(a.y - b.y) < 1;

// first draw: the drawing centred in the viewport (the legend not counted), zoom 100%
// -- where the drawing's centre falls, from the viewport's inner centre
const centre_off = `(() => { const el = document.getElementById("graph"), t = d3.zoomTransform(el);
  return {dx: Math.round(t.applyX(drawing_size.w / 2) - el.clientWidth / 2), dy: Math.round(t.applyY(drawing_size.h / 2) - el.clientHeight / 2), k: t.k}; })()`;
const first = await ev(centre_off);
check(await ev(`legend_w`) > 100, "the legend takes a column: " + await ev(`legend_w`));
check(Math.abs(first.dx) <= 1 && Math.abs(first.dy) <= 1 && first.k === 1, "first draw: centred, zoom 100%: " + JSON.stringify(first));
await ev(`document.getElementById("show-all").click()`); await sleep(1200);
const SUB = "session:1:sub:0";
// the page itself never moves for the graph's sake
const page = async () => ev(`({sx: window.scrollX, sy: window.scrollY, btn: Math.round(document.getElementById("refresh").getBoundingClientRect().left)})`);
const page0 = await page();

// 1. open the sub box by clicking its label: it stays put
let before = await at(SUB), other_before = await at("server");
await click_at(await center(`${G(SUB)}.querySelector(":scope > text.label")`)); await sleep(1000);
let after = await at(SUB);
check(await ev(`expanded.has("${SUB}")`), "the sub box opened");
check(same(before, after), `sub box stays put opening it: ${JSON.stringify(before)} -> ${JSON.stringify(after)}`);

// 2. its endpoint_ triangle: /demo/ hides -- the layout changes, the sub stays
before = await at(SUB);
const others0 = await ev(`[...document.querySelectorAll("g.node")].map(g => g.__data__.id).sort().join(",")`);
await click_at(await center(`[...${G(SUB)}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === "endpoint_").querySelector("tspan.rtoggle")`)); await sleep(1000);
after = await at(SUB);
check(!(await ev(`shown_box_ids.has("stream:/demo/")`)), "endpoint_'s ▾ hid /demo/");
check(same(before, after), `sub box stays put: ${JSON.stringify(before)} -> ${JSON.stringify(after)}`);
// and back
before = after;
await click_at(await center(`[...${G(SUB)}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === "endpoint_").querySelector("tspan.rtoggle")`)); await sleep(1000);
after = await at(SUB);
check(await ev(`shown_box_ids.has("stream:/demo/")`) && same(before, after), `▸ shows /demo/ again; sub box stays put: ${JSON.stringify(before)} -> ${JSON.stringify(after)}`);

// 3. the server box, opened from its label: it stays put while the graph grows beneath
before = await at("server");
await click_at(await center(`${G("server")}.querySelector(":scope > text.label")`)); await sleep(1000);
after = await at("server");
check(same(before, after), `server stays put opening it: ${JSON.stringify(before)} -> ${JSON.stringify(after)}`);
// Fit first: the drawing is wider than the viewport, and the sessions sit
// past its right edge -- a real click there would miss
await ev(`document.getElementById("fit").click()`); await sleep(700);
check(await ev(`(() => { const v = document.getElementById("graph").getBoundingClientRect(), r = ${G("session:2")}.getBoundingClientRect(); return r.right <= v.right && r.left >= v.left; })()`), "session 2 inside the viewport");
// a session's children triangle (outside the box, still its anchor)
before = await at("session:2");
await click_at(await center(`${G("session:2")}.querySelector(":scope > text.kids")`)); await sleep(1000);
after = await at("session:2");
check(same(before, after), `session 2 stays put toggling its children: ${JSON.stringify(before)} -> ${JSON.stringify(after)}`);
// a menu item: Hide children from session 1's menu button
before = await at("session:1");
await click_at(await center(`${G("session:1")}.querySelector(":scope > g.mbtn > rect")`)); await sleep(200);
await ev(`[...document.querySelectorAll("#ctxmenu button")].find(b => b.textContent.startsWith("Hide children")).click()`); await sleep(1000);
after = await at("session:1");
check(!(await ev(`shown_box_ids.has("${SUB}")`)) && same(before, after), `menu "Hide children" on session 1: it stays put: ${JSON.stringify(before)} -> ${JSON.stringify(after)}`);

// 3b. Refresh (no anchor): nothing jumps -- the shift and room are kept
before = await at("session:1");
await ev(`document.getElementById("refresh").click()`); await sleep(1200);
check(same(before, await at("session:1")), `Refresh leaves the boxes where they were: ${JSON.stringify(before)} -> ${JSON.stringify(await at("session:1"))}`);

check(JSON.stringify(await page()) === JSON.stringify(page0), "through all of it the page never scrolled, the controls never moved: " + JSON.stringify(await page()));

// 4. pan: drag the background (not a box)
const vb = await ev(`(() => { const r = document.getElementById("graph").getBoundingClientRect(); return {x: r.right - 30, y: r.bottom - 30}; })()`);
// without Shift a drag is the browser's: no pan
before = await at("server");
await mouse("mouseMoved", vb.x, vb.y, "none"); await mouse("mousePressed", vb.x, vb.y, "left");
for (let i = 1; i <= 5; i++) await mouse("mouseMoved", vb.x - 20 * i, vb.y - 10 * i, "left");
await mouse("mouseReleased", vb.x - 100, vb.y - 50, "left"); await sleep(200);
check(same(before, await at("server")), "a drag WITHOUT Shift does not pan");
// with Shift: pans
before = await at("server");
await mouse("mouseMoved", vb.x, vb.y, "none", SHIFT); await mouse("mousePressed", vb.x, vb.y, "left", SHIFT);
for (let i = 1; i <= 5; i++) await mouse("mouseMoved", vb.x - 20 * i, vb.y - 10 * i, "left", SHIFT);
await mouse("mouseReleased", vb.x - 100, vb.y - 50, "left", SHIFT); await sleep(200);
after = await at("server");
check(Math.abs(after.x - before.x + 100) < 2 && Math.abs(after.y - before.y + 50) < 2, `Shift + drag on the background pans the graph by the drag: ${JSON.stringify(before)} -> ${JSON.stringify(after)}`);
check(JSON.stringify(await page()) === JSON.stringify(page0), "... and not the page");
// a drag starting on a box is not a pan
before = await at("server");
const lbl = await center(`${G("session:2")}.querySelector(":scope > text.label")`);
await mouse("mouseMoved", lbl.x, lbl.y, "none", SHIFT); await mouse("mousePressed", lbl.x, lbl.y, "left", SHIFT);
await mouse("mouseMoved", lbl.x + 60, lbl.y + 30, "left", SHIFT); await mouse("mouseReleased", lbl.x + 60, lbl.y + 30, "left", SHIFT); await sleep(200);
check(same(before, await at("server")), "Shift + drag starting on a box does not pan");
// wheel zooms
const k0 = await ev(`d3.zoomTransform(document.getElementById("graph")).k`);
// without Shift the wheel is the page's: it scrolls, no zoom
await call("Input.dispatchMouseEvent", {type: "mouseWheel", x: vb.x, y: vb.y, deltaX: 0, deltaY: 200}); await sleep(300);
const sy = await ev(`window.scrollY`);
check(sy > 0 && await ev(`d3.zoomTransform(document.getElementById("graph")).k`) === k0, "the wheel WITHOUT Shift scrolls the page (scrollY " + sy + "), no zoom");
await ev(`window.scrollTo(0, 0); 1`); await sleep(200);
await call("Input.dispatchMouseEvent", {type: "mouseWheel", x: vb.x, y: vb.y, deltaX: 0, deltaY: -300, modifiers: SHIFT}); await sleep(300);
const k1 = await ev(`d3.zoomTransform(document.getElementById("graph")).k`);
check(k1 > k0, `Shift + wheel zooms: k ${k0} -> ${k1}`);
// a browser that reports Shift+wheel as sideways (deltaX): still zooms
await call("Input.dispatchMouseEvent", {type: "mouseWheel", x: vb.x, y: vb.y, deltaX: -300, deltaY: 0, modifiers: SHIFT}); await sleep(300);
const k1x = await ev(`d3.zoomTransform(document.getElementById("graph")).k`);
check(k1x > k1, `... also when reported as deltaX: k ${k1} -> ${k1x}`);
check(await ev(`document.getElementById("zoom-level").textContent`) === `zoom ${Math.round(k1x * 100)}%`, "the zoom readout follows: " + await ev(`document.getElementById("zoom-level").textContent`));
// Fit: the whole drawing in view
await ev(`document.getElementById("fit").click()`); await sleep(700);
// the sheet (drawing_size) on screen, in the viewport's inner coordinates
const fit = await ev(`(() => { const el = document.getElementById("graph"), t = d3.zoomTransform(el), W = el.clientWidth, H = el.clientHeight;
  const x0 = t.x, y0 = t.y, x1 = t.x + t.k * drawing_size.w, y1 = t.y + t.k * drawing_size.h;
  return {in: x0 >= -0.5 && y0 >= -0.5 && x1 <= W + 0.5 && y1 <= H + 0.5,
          fills: t.k === 3 || Math.abs(x1 - x0 - W) < 1 || Math.abs(y1 - y0 - H) < 1 || Math.abs(x1 - x0 - (W - legend_w)) < 1 || Math.abs(y1 - y0 - (H - legend_h)) < 1,
          clear: legend_w === 0 || x0 >= legend_w - 0.5 || y0 >= legend_h - 0.5, k: t.k}; })()`);
check(fit.in && fit.fills && fit.clear, "Fit: the whole drawing in view, as large as fits, clear of the legend: " + JSON.stringify(fit));

// 4b. the drawing can't be panned or zoomed out of view: a box centre stays inside
const centres_in = async () => ev(`(() => { const v = document.getElementById("graph").getBoundingClientRect();
  return [...document.querySelectorAll("g.node")].filter(g => { const r = g.querySelector(":scope > rect").getBoundingClientRect();
    const cx = r.left + r.width / 2, cy = r.top + r.height / 2; return cx >= v.left - 1 && cx <= v.right + 1 && cy >= v.top - 1 && cy <= v.bottom + 1; }).length; })()`);
const drag = async (from, dx, dy, steps = 10) => { await mouse("mouseMoved", from.x, from.y, "none", SHIFT); await mouse("mousePressed", from.x, from.y, "left", SHIFT);
  for (let i = 1; i <= steps; i++) await mouse("mouseMoved", from.x + dx * i / steps, from.y + dy * i / steps, "left", SHIFT);
  await mouse("mouseReleased", from.x + dx, from.y + dy, "left", SHIFT); await sleep(150); };
const bg = await ev(`(() => { const r = document.getElementById("graph").getBoundingClientRect(); return {x: r.right - 20, y: r.bottom - 20}; })()`);
// drag far up-left, repeatedly: everything would leave the top-left corner
for (let i = 0; i < 4; i++) await drag(bg, -1300, -800);
const n_in = await centres_in();
check(n_in >= 1, "dragged far off: still a box centre in view (" + n_in + ")");
const edge = await ev(`(() => { const v = document.getElementById("graph").getBoundingClientRect();
  const c = [...document.querySelectorAll("g.node")].map(g => { const r = g.querySelector(":scope > rect").getBoundingClientRect(); return {x: r.left + r.width / 2, y: r.top + r.height / 2}; });
  return c.some(p => Math.abs(p.x - v.left) < 2 || Math.abs(p.y - v.top) < 2); })()`);
check(edge, "... the last one stuck at the viewport's edge");
// a text selection in the graph (the no-Shift drag above makes one): a
// Shift-drag pan must not extend it -- dragging toward the window's bottom
// edge would then scroll the page
check(await ev(`String(getSelection()).length > 0 || (() => { const r = document.createRange(); r.selectNodeContents(document.querySelector("g.node text.label")); getSelection().addRange(r); return String(getSelection()).length > 0; })()`), "some graph text selected");
// dragging back moves at once
before = await at("server");
await drag(bg, 40, 30, 4);
after = await at("server");
check(await ev(`scrollY`) === 0 && String(await ev(`String(getSelection())`)) === "", "the Shift-drag: page not scrolled, the selection dropped");
check(Math.abs(after.x - before.x - 40) < 2 && Math.abs(after.y - before.y - 30) < 2, `dragging back moves at once: ${JSON.stringify(before)} -> ${JSON.stringify(after)}`);
await ev(`document.getElementById("fit").click()`); await sleep(700);
// zoom in hard on an empty corner: the drawing does not vanish
for (let i = 0; i < 8; i++) { await call("Input.dispatchMouseEvent", {type: "mouseWheel", x: bg.x, y: bg.y, deltaX: 0, deltaY: -500, modifiers: SHIFT}); await sleep(60); }
await sleep(300);
check(await centres_in() >= 1, "zoomed in hard on an empty corner: a box centre still in view (zoom " + await ev(`document.getElementById("zoom-level").textContent`) + ")");

// 4c. Center: the drawing's centre at the viewport's (the legend not counted), zoom unchanged
await call("Input.dispatchMouseEvent", {type: "mouseWheel", x: bg.x, y: bg.y, deltaX: 0, deltaY: 200, modifiers: SHIFT}); await sleep(300);
await drag(bg, -150, -90);
const kc = await ev(`d3.zoomTransform(document.getElementById("graph")).k`);
await ev(`document.getElementById("center").click()`); await sleep(700);
const ctr = await ev(`(() => { const v = document.getElementById("graph").getBoundingClientRect();
  const t = d3.zoomTransform(document.getElementById("graph"));
  // the drawing's centre, in drawing coordinates, mapped to the screen
  const cx = v.left + t.applyX(drawing_size.w / 2), cy = v.top + t.applyY(drawing_size.h / 2);
  return {dx: Math.round(cx - (v.left + v.width / 2)), dy: Math.round(cy - (v.top + v.height / 2)), k: t.k}; })()`);
check(Math.abs(ctr.dx) <= 1 && Math.abs(ctr.dy) <= 1 && ctr.k === kc, "Center: the drawing's centre at the viewport's, zoom kept: " + JSON.stringify(ctr) + " k was " + kc);

// 5. Show all / Hide all reset the camera: centred, 100%
await ev(`document.getElementById("show-all").click()`); await sleep(1200);
let rc = await ev(centre_off);
check(Math.abs(rc.dx) <= 1 && Math.abs(rc.dy) <= 1 && rc.k === 1, "Show all: the drawing centred, 100%: " + JSON.stringify(rc));
check(await ev(`document.getElementById("zoom-level").textContent`) === "zoom 100%", "... and the readout says 100%");
await ev(`d3.select("#graph").call(zoom.transform, d3.zoomIdentity.translate(-200, 90).scale(0.6)); 1`); await sleep(200);
await ev(`document.getElementById("hide-all").click()`); await sleep(1200);
rc = await ev(centre_off);
check(Math.abs(rc.dx) <= 1 && Math.abs(rc.dy) <= 1 && rc.k === 1, "Hide all (from elsewhere, 60%): centred, 100%: " + JSON.stringify(rc));
await ev(`document.getElementById("show-all").click()`); await sleep(1200);
// the viewport fills the window below the controls
const vp = await ev(`(() => { const r = document.getElementById("graph").getBoundingClientRect(); return {bottom: Math.round(r.bottom), win: window.innerHeight, w: Math.round(r.width), page_w: document.documentElement.clientWidth}; })()`);
check(vp.win - vp.bottom >= 0 && vp.win - vp.bottom < 30 && vp.page_w - vp.w < 60, "the viewport fills the window below the controls: " + JSON.stringify(vp));
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
