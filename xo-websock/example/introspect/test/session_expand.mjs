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
const mouse = async (type, x, y, button) => call("Input.dispatchMouseEvent", {type, x, y, button, clickCount: 1});
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok = " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1400, height: 900, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`!!last_event && document.querySelectorAll("g.node").length >= 1`)); i++) await sleep(100);
await sleep(300);
// only the Webserver box is shown by default: show every box's children
await ev(`document.getElementById("show-all").click()`);
for (let i = 0; i < 100 && !(await ev(`document.querySelectorAll("path.edge").length > 0`)); i++) await sleep(100);
await sleep(500);
// Show all centres the drawing at 100%: wider than the viewport, its sides
// are off screen.  Fit, so the boxes clicked below are in view
await ev(`document.getElementById("fit").click()`); await sleep(800);
const sel = `[...document.querySelectorAll("g.node.session")].find(g => g.querySelector("text.sub").textContent.includes("session 1"))`;
check(await ev(`${sel}.classList.contains("expandable")`), "a session box is expandable");
const b = await ev(`(() => { const r = ${sel}.querySelector("rect").getBoundingClientRect(); return {x: r.left + 20, y: r.top + 15}; })()`);
await mouse("mouseMoved", b.x, b.y, "none"); await mouse("mousePressed", b.x, b.y, "left"); await mouse("mouseReleased", b.x, b.y, "left");
await sleep(800);
const rows = await ev(`[...${sel}.querySelectorAll("text.row")].map(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join(""))`);
console.log(" =  rows:", JSON.stringify(rows));
// reflected members first (router_), then the rest (xo-printjson#06)
// output_buf_: a raw pointer, borrowed -- a ref, its OutputBuffer unplaced
// (xo-printjson#08), until xo-websock#15 decides where it prints
check(rows.length === 4 && rows[1] === "output_buf_: (→ not drawn)", "4 rows; output_buf_ a ref, not drawn: " + rows[1]);
check(rows[2] === "sender_: ▾ (→)", "sender_ a ref: " + rows[2]);
check(rows[3] === "outbound_q_: 0 queued", "outbound_q_ mentioned: " + rows[3]);
check(await ev(`${VIS}.length`) === 2, "two member edges: sender_, and router_ -- its own (nested) box");
const geo = await ev(`(() => { const p = ${VIS}.find(p => p.__data__.row_keys.includes("session:1/sender_")).getAttribute("d"); const n = p.match(/-?[0-9.]+/g).map(Number);
   const ex = n[n.length-2], ey = n[n.length-1];
   const snd = document.querySelector("g.node.sender"); // the expanded session's sender:
   const senders = [...document.querySelectorAll("g.node.sender")].map(g => { const t = g.transform.baseVal[0].matrix, r = g.querySelector("rect"); return {x: t.e, y: t.f, w: +r.getAttribute("width"), h: +r.getAttribute("height"), id: g.__data__.id}; });
   const hit = senders.find(s => ex >= s.x - 2 && ex <= s.x + s.w + 2 && ey >= s.y - 2 && ey <= s.y + s.h + 2);
   return hit ? hit.id : null; })()`);
check(geo === "session:1:sender", `member edge ends at session 1's sender box: ${geo}`);
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png", clip: {x: 0, y: 0, width: 1400, height: 900, scale: 1}})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
