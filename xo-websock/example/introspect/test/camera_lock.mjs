// 🔒 beside Fit / Center: while locked, every redraw ends as pressing that
// button would; locking one releases the other; a pan or zoom gesture
// releases it; a resize re-applies it
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
const cam = () => ev(`(() => { const t = d3.zoomTransform(graph_svg.node()); return {x: t.x, y: t.y, k: t.k}; })()`);
const want = (expr) => ev(`(() => { const t = ${expr}; return {x: t.x, y: t.y, k: t.k}; })()`);
const near = (a, b) => a && b && Math.abs(a.x - b.x) < 1 && Math.abs(a.y - b.y) < 1 && Math.abs(a.k - b.k) < 1e-3;
const pressed = (id) => ev(`document.getElementById(${JSON.stringify(id)})?.getAttribute("aria-pressed")`);
const settle = async () => { await ev(`settled()`); await sleep(150); };
check(await pressed("fit-lock") === "false" && await pressed("center-lock") === "false", "🔒 beside Fit and Center, both off");
check(await ev(`document.getElementById("fit-lock").textContent`) === "🔒", "the lock is 🔒");
// Fit lock: on at once, then after every redraw
await ev(`document.getElementById("fit-lock").click(); 1`); await sleep(600);   // its own camera move: not a redraw
check(await pressed("fit-lock") === "true", "Fit locked: pressed");
check(near(await cam(), await want(`fit_camera(drawing_size)`)), "locking applies Fit at once");
const size0 = await ev(`({...drawing_size})`);
await ev(`${G("server")}.dispatchEvent(new MouseEvent("click", {bubbles: true})); 1`); await settle();
check(JSON.stringify(await ev(`({...drawing_size})`)) !== JSON.stringify(size0), "opening the server changed the drawing");
check(near(await cam(), await want(`fit_camera(drawing_size)`)), "a redraw ends fitted, not anchored: " + JSON.stringify([await cam(), await want(`fit_camera(drawing_size)`)]));
// resize: re-applied
await call("Emulation.setDeviceMetricsOverride", {width: 1200, height: 800, deviceScaleFactor: 1, mobile: false}); await sleep(700);
check(near(await cam(), await want(`fit_camera(drawing_size)`)), "a resize re-applies Fit");
// Center lock: releases Fit; keeps the zoom
await ev(`document.getElementById("center-lock").click(); 1`); await sleep(600);
check(await pressed("center-lock") === "true" && await pressed("fit-lock") === "false", "Center locked; Fit released");
const k0 = (await cam()).k;
await ev(`document.getElementById("show-all").click(); 1`); await settle();
check(near(await cam(), await want(`centred(drawing_size, ${k0})`)), "Show all ends centred at the zoom it had: " + JSON.stringify([await cam(), k0]));
// a gesture releases it
const vb = await ev(`(() => { const r = document.getElementById("graph").getBoundingClientRect(); return {x: r.right - 30, y: r.bottom - 30}; })()`);
await call("Input.dispatchMouseEvent", {type: "mouseWheel", x: vb.x, y: vb.y, deltaX: 0, deltaY: -300, modifiers: 8}); await sleep(300);
check(await pressed("center-lock") === "false", "a Shift + wheel zoom releases the lock");
const after_zoom = await cam();
await ev(`document.getElementById("refresh").click(); 1`); await settle();
check(near(await cam(), after_zoom), "released: a redraw leaves the camera where the gesture put it");
await call("Emulation.setDeviceMetricsOverride", {width: 1500, height: 1000, deviceScaleFactor: 1, mobile: false});
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); process.exit(ok ? 0 : 1);
