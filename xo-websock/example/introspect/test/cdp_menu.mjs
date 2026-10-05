// drive the introspect page in headless chrome: real right-clicks (CDP Input),
// then inspect the DOM
const [,, cdp_port, page_url] = process.argv;
const tgt = await (await fetch(`http://localhost:${cdp_port}/json/new?${page_url}`, {method: "PUT"})).json();
const ws = new WebSocket(tgt.webSocketDebuggerUrl);
await new Promise(r => ws.onopen = r);
let seq = 0; const pending = new Map();
ws.onmessage = (ev) => { const m = JSON.parse(ev.data); if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); } };
const call = (method, params = {}) => new Promise(r => { const id = ++seq; pending.set(id, r); ws.send(JSON.stringify({id, method, params})); });
const ev = async (expr) => { const r = await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true}); return r.result.result.value; };
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
const mouse = async (type, x, y, button) => call("Input.dispatchMouseEvent", {type, x, y, button, clickCount: 1});
const right_click = async (x, y) => { await mouse("mouseMoved", x, y, "none"); await mouse("mousePressed", x, y, "right"); await mouse("mouseReleased", x, y, "right"); };
const key = async (k, code) => { for (const type of ["keyDown", "keyUp"]) await call("Input.dispatchKeyEvent", {type, key: k, code: code || k, windowsVirtualKeyCode: {Escape: 27, ArrowDown: 40, ContextMenu: 93, Tab: 9}[k] || 0}); };

let ok = true; const check = (c, msg) => { console.log(`${c ? "ok = " : "FAIL"} ${msg}`); if (!c) ok = false; };

for (let i = 0; i < 100 && !(await ev(`!!last_event && document.querySelectorAll("g.node").length >= 1`)); i++) await sleep(100);
// only the Webserver box is shown by default: show every box's children
await ev(`document.getElementById("show-all").click()`);
for (let i = 0; i < 100 && !(await ev(`document.querySelectorAll("path.edge").length > 0`)); i++) await sleep(100);
await sleep(500);   // /dyn/types loaded
check(await ev(`document.querySelectorAll("g.node").length > 2`), "graph drawn");
// the window is small (headless default): Fit, so the boxes clicked are in view
await ev(`document.getElementById("fit").click()`); await sleep(800);

// the box's RECT (the g's bounds include a refcount badge poking above it)
const box = async (sel) => ev(`(() => { const r = document.querySelector(${JSON.stringify(sel)} + " > rect").getBoundingClientRect(); return {x: r.left + 20, y: r.top + r.height / 2}; })()`);
const menu = async () => ev(`({hidden: document.getElementById("ctxmenu").hidden,
    head: document.querySelector("#ctxmenu .ctxhead")?.textContent,
    items: [...document.querySelectorAll("#ctxmenu button")].map(b => b.textContent + (b.disabled ? " (disabled: " + b.title + ")" : "")),
    focus: document.activeElement?.textContent})`);

// 1. right-click the server box
let b = await box("g.node.server");
await right_click(b.x, b.y);
await sleep(100);
let m = await menu();
check(!m.hidden, `right-click on server opens menu: ${JSON.stringify(m)}`);
check(m.head === "xo::web::WebserverImpl", "menu header is the type");
// no link provider in this run: Open source is disabled, so the first ENABLED item has focus
// the server box has members: Expand first; no link provider: Open source disabled
check(m.items[0] === "Expand", "Expand first, enabled");
// every child drawn (Show all): Hide children, its count the server's children
const n_kids = await ev(`[...document.querySelectorAll("g.node")].find(g => g.__data__.id === "server").__data__.n_children`);
const hide_kids = `Hide children (-${n_kids})`;
check(m.items[1] === hide_kids, `then ${hide_kids}: Show all drew them all, so no Show children: ` + m.items[1]);
check(m.items[2] === "Show types (disabled: no member rows shown)", "then Show types, disabled while collapsed: " + m.items[2]);
check(/^Hide Webserver :\d+ \(running\) \(disabled/.test(m.items[3]), "then Hide <its label> (disabled: the Webserver is always shown): " + m.items[3]);
check(m.items[4] === "Hide ▸ WebserverConfig", "then a Hide ▸ per drawn child (its nested boxes, first ws_config_): " + m.items[4]);
const last_child = m.items.findLastIndex(t => t.startsWith("Hide ▸ "));
check(m.items[last_child + 1].startsWith("Open source (disabled"), "Open source disabled without a link provider");
check(m.focus === "Expand", "first enabled item focused");

// 2. arrow keys skip disabled items; Enter on Show JSON
await key("ArrowDown");
m = await menu();
check(m.focus === hide_kids, "ArrowDown: the next item: " + m.focus);
await key("ArrowDown");
m = await menu();
check(m.focus === "Hide ▸ WebserverConfig", "ArrowDown skips the disabled Show types and Hide: " + m.focus);
await key("ArrowUp");
m = await menu();
check(m.focus === hide_kids, "ArrowUp moves back: " + m.focus);
for (let i = 0; i < 20 && m.focus !== m.items[last_child]; i++) { await key("ArrowDown"); m = await menu(); }
await key("ArrowDown");
m = await menu();
check(m.focus === "Show JSON", "ArrowDown from the last child skips the disabled Open source: " + m.focus);
await call("Input.dispatchKeyEvent", {type: "keyDown", key: "Enter", code: "Enter", windowsVirtualKeyCode: 13, text: "\r"});
await call("Input.dispatchKeyEvent", {type: "keyUp", key: "Enter", code: "Enter", windowsVirtualKeyCode: 13});
await sleep(100);
const det = await ev(`({h: document.getElementById("detail-h").textContent, hidden: document.getElementById("detail").hidden,
    id: JSON.parse(document.getElementById("detail").textContent || "{}").id, srv: null})`);
check(!det.hidden && det.h.startsWith("object: Webserver"), `Show JSON shows the object: ${det.h}`);
check((await menu()).hidden, "menu closed after choosing");
await sleep(1000);   // Show JSON scrolled the page (smoothly): let it settle

// 3. Escape closes; click outside closes.  Show JSON scrolled down to the
// object: back to the graph first, as a person would
await ev("window.scrollTo(0, 0)");
await ev(`document.getElementById("fit").click()`);   // nested boxes widen the drawing past the viewport
await sleep(800);
b = await box("g.node.http");
await right_click(b.x, b.y);
await sleep(50);
check(!(await menu()).hidden, "menu opens on an endpoint");
await key("Escape");
check((await menu()).hidden, "Escape closes it");
await right_click(b.x, b.y);
await mouse("mousePressed", 5, 5, "left"); await mouse("mouseReleased", 5, 5, "left");
check((await menu()).hidden, "a click elsewhere closes it");

// 4. right-click on blank svg: page does not take it over
const prevented_blank = await ev(`(() => { const svg = document.getElementById("graph");
    const e = new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: svg.getBoundingClientRect().right - 5, clientY: svg.getBoundingClientRect().bottom - 5});
    svg.dispatchEvent(e); return e.defaultPrevented; })()`);
check(!prevented_blank && (await menu()).hidden, "blank canvas: browser's menu left alone");

// 5. keyboard: focus a box, Shift+F10 / Menu key -> contextmenu at the box
// a Menu key sent over CDP does not make headless chrome fire contextmenu, so
// dispatch what a keyboard-initiated one looks like: no pointer position
await ev(`(() => { const g = document.querySelector("g.node.session"); g.focus();
    g.dispatchEvent(new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: 0, clientY: 0})); })()`);
await sleep(100);
m = await menu();
check(!m.hidden && m.head && m.head.includes("WebsocketSessionRecd"), `Menu key on focused session opens its menu: ${m.head}`);
const pos = await ev(`(() => { const r = document.querySelector("g.node.session").getBoundingClientRect(); const mm = document.getElementById("ctxmenu").getBoundingClientRect(); return {box_bottom: r.bottom, menu_top: mm.top, box_left: r.left, menu_left: mm.left}; })()`);
check(Math.abs(pos.menu_top - pos.box_bottom) < 30 && Math.abs(pos.menu_left - pos.box_left) < 40, `keyboard menu placed at the box: ${JSON.stringify(pos)}`);
await key("Escape");
check(await ev(`document.activeElement === document.querySelector("g.node.session")`), "Escape returns focus to the box");

// 6. copy id: clipboard (localhost is a secure context)
b = await box("g.node.server");
await right_click(b.x, b.y);
await ev(`[...document.querySelectorAll("#ctxmenu button")].find(x => x.textContent === "Copy type name").click()`);
check((await menu()).hidden, "Copy type name closes the menu");

// 7. the menu button: left-click opens the box menu, not a toggle
const left_click = async (x, y) => { await mouse("mouseMoved", x, y, "none"); await mouse("mousePressed", x, y, "left"); await mouse("mouseReleased", x, y, "left"); };
// the graph has its own viewport: bring every box into it before clicking
await ev(`document.getElementById("fit").click(); 1`); await sleep(700);
const btn = await ev(`(() => { const r = document.querySelector("g.node.session > g.mbtn > rect").getBoundingClientRect(); return {x: r.left + r.width / 2, y: r.top + r.height / 2, w: r.width, h: r.height}; })()`);
const bsz = await ev(`(() => { const r = document.querySelector("g.node.session > g.mbtn > rect"); return {w: +r.getAttribute("width"), h: +r.getAttribute("height")}; })()`);
check(bsz.w > 15 && bsz.w === bsz.h, "every box has a square menu button (drawn size, whatever the zoom): " + JSON.stringify(bsz));
check(await ev(`[...document.querySelectorAll("g.node")].every(g => g.querySelector(":scope > g.mbtn"))`), "... on every box");
check(!(await ev(`document.querySelector("g.node.session > text.label").textContent.includes("⋯")`)), "no ⋯ in the label any more");
const was_open = await ev(`expanded.has(document.querySelector("g.node.session").__data__.id)`);
await left_click(btn.x, btn.y); await sleep(200);
m = await menu();
check(!m.hidden && m.head.includes("WebsocketSessionRecd"), "left-click on the button opens the box menu: " + m.head);
check(await ev(`expanded.has(document.querySelector("g.node.session").__data__.id)`) === was_open, "... and does not open or close the box");
const below = await ev(`(() => { const r = document.querySelector("g.node.session > g.mbtn > rect").getBoundingClientRect(), mm = document.getElementById("ctxmenu").getBoundingClientRect(); return {dx: mm.left - r.left, dy: mm.top - r.bottom}; })()`);
check(Math.abs(below.dx) < 2 && below.dy >= 0 && below.dy < 6, "menu just below the button: " + JSON.stringify(below));
await key("Escape"); await sleep(100);
// left-click elsewhere on the box still toggles it
const lbl = await ev(`(() => { const r = document.querySelector("g.node.session > text.label").getBoundingClientRect(); return {x: r.left + 5, y: r.top + r.height / 2}; })()`);
await left_click(lbl.x, lbl.y); await sleep(900);
check(await ev(`expanded.has(document.querySelector("g.node.session").__data__.id)`) !== was_open, "left-click on the label toggles the box");
check((await menu()).hidden, "... with no menu");

// 8. per-box types: expand the server, then Show types from its menu
const srv_rows = async () => ev(`[...document.querySelector("g.node.server").querySelectorAll("text.row")].map(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join(""))`);
// right-click the header (mid-box is a member row once expanded: its own menu)
const srv_menu = async () => { const p = await ev(`(() => { const r = document.querySelector("g.node.server > rect").getBoundingClientRect(); return {x: r.left + 20, y: r.top + 8}; })()`); await right_click(p.x, p.y); await sleep(100); return menu(); };
const choose = async (label) => { await ev(`[...document.querySelectorAll("#ctxmenu button")].find(x => x.textContent === ${JSON.stringify(label)}).click()`); await sleep(900); };
await ev(`document.getElementById("fit").click(); 1`); await sleep(700);
if (!(await ev(`expanded.has("server")`))) { await srv_menu(); await choose("Expand"); }
await ev(`document.getElementById("fit").click(); 1`); await sleep(700);
let rr = await srv_rows();
check(rr.length > 0 && rr.every(t => !/ \[[a-z]+\]/.test(t)), "expanded server: rows without types: " + JSON.stringify(rr.slice(0, 2)));
m = await srv_menu();
const after_kids = (items) => items[items.findLastIndex(t => / children/.test(t)) + 1];
check(after_kids(m.items) === "Show types", "expanded: Show types enabled, just under the children items: " + JSON.stringify(m.items.slice(0, 4)));
await choose("Show types");
rr = await srv_rows();
check(rr.includes("listen_port_: atomic<int> [atomic]") && rr.includes("ws_config_: ▾ WebserverConfig [struct] (→)"), "Show types: name: Type [metatype], a ref keeping its arrow: " + JSON.stringify(rr.slice(0, 2)));
const others = await ev(`[...document.querySelectorAll("g.node:not(.server) text.row")].map(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join(""))`);
check(others.every(t => !/ \[[a-z]+\]/.test(t)), `only the server shows types (${others.length} rows elsewhere)`);
m = await srv_menu();
check(after_kids(m.items) === "Hide types", "the item flips to Hide types: " + after_kids(m.items));
await choose("Hide types");
rr = await srv_rows();
check(rr.every(t => !/ \[[a-z]+\]/.test(t)) && rr.some(t => /^listen_port_: \d+$/.test(t)), "Hide types: plain rows again");

console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); process.exit(ok ? 0 : 1);
