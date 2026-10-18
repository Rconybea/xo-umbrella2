// a box's ⋯ button: hovering opens its menu (after a short delay, without
// taking focus); the menu stays while the pointer is on the button or the
// menu, and closes once it leaves both.  A click opens it at once and pins it
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
// headless chrome blurs the window ~2s after load, and a window blur closes
// the menu (as leaving the window should): keep the page focused
await call("Emulation.setFocusEmulationEnabled", {enabled: true});
const mouse = (type, p, button = "none") => call("Input.dispatchMouseEvent", {type, x: p.x, y: p.y, button, clickCount: 1});
const center = (sel) => ev(`(() => { const r = ${sel}.getBoundingClientRect(); return {x: r.left + r.width / 2, y: r.top + r.height / 2}; })()`);
const BTN = `document.querySelector("g.node.server > g.mbtn > rect")`;
const open = () => ev(`!document.getElementById("ctxmenu").hidden`);
const focus_in_menu = () => ev(`document.getElementById("ctxmenu").contains(document.activeElement)`);
const away = {x: 1400, y: 950};   // the svg's empty corner, far from the server box
const b = await center(BTN);
check(await ev(`!!document.elementFromPoint(${b.x}, ${b.y}).closest("g.node.server > g.mbtn")`), "the button is under the pointer's spot: " + JSON.stringify(b));
check(!(await ev(`!!document.querySelector("g.node.server > g.mbtn > title") || document.querySelector("g.node.server > g.mbtn").hasAttribute("data-tip")`)), "no \"menu\" tooltip");
// 1. hover: not at once, then open, without taking focus
await mouse("mouseMoved", away); await sleep(100);
await mouse("mouseMoved", b); await sleep(80);
check(!(await open()), "hover: not open at once");
await sleep(500);
check(await open(), "hover: open after the delay");
check(!(await focus_in_menu()), "hover: the menu does not take focus");
// 2. into the menu: stays open
const item = await center(`document.querySelector("#ctxmenu button")`);
await mouse("mouseMoved", {x: b.x, y: (b.y + item.y) / 2}); await mouse("mouseMoved", item); await sleep(600);
check(await open(), "the pointer moved into the menu: still open");
// 3. away from both: closes
await mouse("mouseMoved", away); await sleep(600);
check(!(await open()), "the pointer left button and menu: closed");
// 4. click: open at once, focused, pinned
await mouse("mouseMoved", b); await mouse("mousePressed", b, "left"); await mouse("mouseReleased", b, "left"); await sleep(50);
check(await open(), "click: open at once");
check(await focus_in_menu(), "click: its first item focused");
await mouse("mouseMoved", away); await sleep(600);
check(await open(), "click: pinned -- leaving does not close it");
await call("Input.dispatchKeyEvent", {type: "keyDown", key: "Escape", code: "Escape", windowsVirtualKeyCode: 27});
await call("Input.dispatchKeyEvent", {type: "keyUp", key: "Escape", code: "Escape", windowsVirtualKeyCode: 27}); await sleep(100);
check(!(await open()), "Escape closes it");
// 5. only while the drawing is still: a redraw slides boxes under a resting
// pointer -- e.g. a menu item chosen with the pointer still on a button --
// and that is not a hover (anchor.mjs met it: a menu reopened by itself)
await mouse("mouseMoved", away); await sleep(100);
await ev(`redraw(); 1`);
await mouse("mouseMoved", b); await sleep(600);
check(!(await open()), "reached while the drawing moves: no menu");
await ev(`settled()`);
await mouse("mouseMoved", away); await sleep(50);
await mouse("mouseMoved", b); await sleep(600);
check(await open(), "the drawing still again: a hover opens it");
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); process.exit(ok ? 0 : 1);
