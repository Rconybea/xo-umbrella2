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
const S1 = `[...document.querySelectorAll("g.node.session")].find(g => g.querySelector("text.sub").textContent.includes("session 1"))`;
const rows = async () => ev(`[...${S1}.querySelectorAll("text.row")].map(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join(""))`);
const click_row = async (prefix) => ev(`[...${S1}.querySelectorAll("text.row")].find(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join("").trimStart().startsWith(${JSON.stringify(prefix)})).dispatchEvent(new MouseEvent("click", {bubbles: true}))`);
// session 1: its router_ is its own (nested) box -- the row a ref to it
const RB = `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === "session:1/router_")`;
const rows_of = async (sel) => ev(`[...${sel}.querySelectorAll("text.row")].map(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join(""))`);
const click_in = async (sel, prefix) => ev(`[...${sel}.querySelectorAll("text.row")].find(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join("").trimStart().startsWith(${JSON.stringify(prefix)})).dispatchEvent(new MouseEvent("click", {bubbles: true}))`);
await ev(`${S1}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(700);
let r = await rows_of(S1);
check(r.some(t => t.trimStart().startsWith("router_:") && t.endsWith("▾ (→)")), "session's router_ row: a ref to its own box");
await ev(`${RB}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(700);
r = await rows_of(RB);
console.log(" =  router rows:", JSON.stringify(r));
check(r.some(t => t.trimStart().startsWith("url_router_:") && t.endsWith("▾ (→)")), "url_router_: a drawn ref (to the UrlRouter box)");
// a unique_ptr to a reflected struct with no members: its name shows presence (xo-reflect#04)
check(r.some(t => t === "readjson_: CharReader"), "readjson_ presence");
check(r.some(t => t.startsWith("subscription_v_:") && t.endsWith("▸ [1]")), "subscription_v_ closed, 1 slot");
await click_in(RB, "subscription_v_:"); await sleep(700);
r = await rows_of(RB);
check(r.some(t => t === "[0]: ▾ (→)"), "slot [0] a ref row");
// edges from the router's box, by where they end
const ends = await ev(`(() => { const boxes = [...document.querySelectorAll("g.node")].map(g => { const t = g.transform.baseVal[0].matrix, x = g.querySelector("rect"); return {id: g.__data__.id, x: t.e, y: t.f, w: +x.getAttribute("width"), h: +x.getAttribute("height")}; });
  return ${VIS}.filter(p => p.__data__.from === "session:1/router_").map(p => { const n = p.getAttribute("d").match(/-?[0-9.]+/g).map(Number); const ex = n[n.length-2], ey = n[n.length-1];
    const b = boxes.find(b => ex >= b.x - 2 && ex <= b.x + b.w + 2 && ey >= b.y - 2 && ey <= b.y + b.h + 2); return b ? b.id : null; }).sort(); })()`);
console.log(" =  router edges end at:", JSON.stringify(ends));
check(JSON.stringify(ends) === JSON.stringify(["server", "server/url_router_", "session:1:sender", "session:1:sub:0"]),
      "router edges: pjson_ -> the server (shared); url_router_ -> the UrlRouter box; sender_ -> its sender; [0] -> its subscription");
// edges into the sender -- the session's sender_, the router's, the
// sink's -- come from three boxes, so not merged; hover pairs each with its row
const into = await ev(`[...document.querySelectorAll("path.edge.member")].filter(p => p.__data__.target === "session:1:sender").map(p => [p.__data__.from, p.__data__.row_keys])`);
check(JSON.stringify(into.sort()) === JSON.stringify([["session:1", ["session:1/sender_"]], ["session:1/router_", ["session:1/router_/sender_"]], ["session:1:sub:0:sink", ["session:1:sub:0:sink/sender_"]]]),
      "three edges into the sender, one per box (session, router, sink): " + JSON.stringify(into));
await ev(`[...document.querySelectorAll("text.row")].find(t => t.__data__.key === "session:1/router_/sender_").dispatchEvent(new MouseEvent("mouseenter"))`); await sleep(100);
let hot = await ev(`({rows: [...document.querySelectorAll("text.row.hot")].map(t => t.__data__.key), edge: [...document.querySelectorAll("path.edge.member.hot")].map(p => p.__data__.from)})`);
check(JSON.stringify(hot) === JSON.stringify({rows: ["session:1/router_/sender_"], edge: ["session:1/router_"]}), "router's sender_ row hover: its own edge and row only: " + JSON.stringify(hot));
await ev(`[...document.querySelectorAll("text.row")].find(t => t.__data__.key === "session:1/router_/sender_").dispatchEvent(new MouseEvent("mouseleave"))`); await sleep(100);
const overl = await ev(`(() => { const r = [...document.querySelectorAll("g.node > rect")].map(e => e.getBoundingClientRect()); let n = 0;
  for (let i = 0; i < r.length; i++) for (let j = i + 1; j < r.length; j++) { const a = r[i], b = r[j];
    if (a.left < b.right && b.left < a.right && a.top < b.bottom && b.top < a.bottom) n++; } return n; })()`);
check(overl === 0, "no overlaps");
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png", clip: {x: 0, y: 0, width: 1500, height: 1000, scale: 1}})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
